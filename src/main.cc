//
//  main.cc
//  Residue
//
//  Copyright 2017-present Muflihun Labs
//
//  Author: @abumusamq
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#include <csignal>
#include <cstdlib>
#include <unistd.h>

#include <chrono>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
#include <unordered_map>
#include <utility>

#include "ripe/Ripe.h"

#include "admin/admin-request-handler.h"
#include "cli/command-handler.h"
#include "connect/connection-request-handler.h"
#include "core/configuration.h"
#include "core/registry.h"
#include "core/residue-exception.h"
#include "crash-handlers.h"
#include "crypto/base64.h"
#include "logging/log-request-handler.h"
#include "logging/log.h"
#include "logging/residue-log-dispatcher.h"
#include "logging/user-log-builder.h"
#include "net/server.h"
#include "setup.h"
#include "tasks/auto-updater.h"
#include "tasks/client-integrity-task.h"
#include "tasks/log-rotator.h"

#ifdef RESIDUE_USE_MINE
#   include "mine/mine.h"
#endif

using namespace residue;
using net::ip::tcp;

INITIALIZE_EASYLOGGINGPP

extern bool s_exitOnInterrupt;

static const std::unordered_map<el::base::type::VerboseLevel, std::string> VERBOSE_SEVERITY_MAP
{
    { RV_CRAZY,   "vCRAZY"   },
    { RV_TRACE,   "vTRACE"   },
    { RV_DEBUG_2, "vDEBUG2"  },
    { RV_DEBUG,   "vDEBUG"   },
    { RV_DETAILS, "vDETAILS" },
    { RV_ERROR,   "vERROR"   },
    { RV_WARNING, "vWARNING" },
    { RV_NOTICE,  "vNOTICE"  },
    { RV_INFO,    "vINFO"    },
    { 0,          ""         }
};

///
/// \brief Used for custom format specifier
///
std::string getVerboseSeverityName(const el::LogMessage* message)
{
    if (message->verboseLevel() <= RV_CRAZY) {
        return VERBOSE_SEVERITY_MAP.at(message->verboseLevel());
    }
    return "";
}

///
/// \brief Configure Easylogging++ to custom settings for residue server
///
el::LogBuilder* configureLogging(Configuration* configuration)
{

    // Note: Do not add StrictLogFileSizeCheck flag as we manually have log rotator in-place

    // %vnamelevel
    el::Helpers::reserveCustomFormatSpecifiers(1);

    // We do not want application to die on FATAL log
    el::Loggers::addFlag(el::LoggingFlag::DisableApplicationAbortOnFatalLog);

    // Client will specify verbosity level
    el::Loggers::addFlag(el::LoggingFlag::AllowVerboseIfModuleNotSpecified);

    if (configuration->hasFlag(Configuration::Flag::IMMEDIATE_FLUSH)) {
        // Immediately flush - dispatch happens in separate thread
        el::Loggers::addFlag(el::LoggingFlag::ImmediateFlush);
    }

    // DefaultLogBuilder will get some values at run time, for example thread, app or even date.
    // We want to log user's values instead hence we create our own log builder and reset storage
    // to use it instead
    el::LogBuilderPtr logBuilder = el::LogBuilderPtr(new UserLogBuilder());
    el::Loggers::setDefaultLogBuilder(logBuilder);

    // update all the loggers (this will include known loggers that were registered at the configuration time)
    // to use this log builder
    std::vector<std::string> registeredLoggers;
    el::Loggers::populateAllLoggerIds(&registeredLoggers);

    for (const std::string& loggerId : registeredLoggers) {
        if (loggerId != RESIDUE_LOGGER_ID) { // all the loggers except for 'residue' uses custom log builder
            el::Loggers::getLogger(loggerId)->setLogBuilder(logBuilder);
        }
    }

    // We use our own log dispatcher as we want to do some checks for safety
    // for missing files etc. This dispatcher also includes updating the file permission to the correct user
    // which is picked up from configuration
    el::Helpers::uninstallLogDispatchCallback<el::base::DefaultLogDispatchCallback>("DefaultLogDispatchCallback");
    el::Helpers::installLogDispatchCallback<ResidueLogDispatcher>("ResidueLogDispatcher");
    ResidueLogDispatcher* dispatcher = el::Helpers::logDispatchCallback<ResidueLogDispatcher>("ResidueLogDispatcher");
    dispatcher->setConfiguration(configuration);

    // do not reconfigure existing loggers as we have configured known loggers already
    // with their respective configurations
    std::string defaultConfigFile = configuration->getConfigurationFile("default");
    el::Configurations defaultLoggerConf = el::Configurations(defaultConfigFile);
    el::Loggers::setDefaultConfigurations(defaultLoggerConf, false);

    return logBuilder.get();
}

void printVersion(bool addSpaces = false)
{
    std::cout <<  (addSpaces ? "  " : "") << "Residue v" << RESIDUE_VERSION;
#ifdef RESIDUE_DEBUG
    std::cout << "-debug";
#endif
#ifdef RESIDUE_HAS_EXTENSIONS
    std::cout << " (with extension support)";
#endif
    std::cout << "\n" << (addSpaces ? "  " : "") << "Built on "
              << __DATE__ << ", " << __TIME__ << std::endl;
}

int main(int argc, char* argv[])
{

    START_EASYLOGGINGPP(argc, argv);
    el::Helpers::setThreadName("MainThread");

    // %vnamelevel
    el::Helpers::installCustomFormatSpecifier(el::CustomFormatSpecifier("%vnamelevel", getVerboseSeverityName));

    // Residue logger is for residue server internal logs / error reports
    el::Logger* residueLogger = el::Loggers::getLogger(RESIDUE_LOGGER_ID);
    if (residueLogger == nullptr) {
        LOG(FATAL) << "Unable to register [residue] logger";
        return 1;
    }

    if (argc < 2) {
        std::cerr << "USAGE: residue <residue_config_file> [--force-without-root] [--v=<verbose-level>] [--residue-home=<new-RESIDUE_HOME>]" << std::endl;
        return 1;
    }

#ifndef ELPP_THREAD_SAFE
    RLOG(WARNING) << "Your residue server is not thread-safe. Please visit https://github.com/muflihun/residue for details.";
#endif

    if (strcmp(argv[1], "--version") == 0 || (strcmp(argv[1], "-v") == 0 && argc == 2)) {
        printVersion();
        return 0;
    } else if (strcmp(argv[1], "--help") == 0) {
        printVersion();
        std::cout << std::endl;
        std::cout << "Please go to https://github.com/muflihun/residue/blob/master/docs/ for help" << std::endl;
        return 0;
    } else if (strcmp(argv[1], "--setup") == 0) {
        return Setup::setup();
    }

    if (!el::Helpers::commandLineArgs()->hasParam("--force-without-root") && el::base::utils::OS::getBashOutput("whoami") != "root") {
        std::cerr << "Please run as 'root'. See https://github.com/muflihun/residue/blob/master/docs/INSTALL.md#run-as-root for more details" << std::endl;
        return 1;
    }

    if (el::Helpers::commandLineArgs()->hasParam("--attach-terminate-handler")) {
        std::cout << "Attaching terminate handler..." << std::endl;
        std::set_terminate(terminateHandler);
    }
    el::Helpers::setCrashHandler(elppCrashHandler);

    std::cout << Base64::decode("ICBfX18gICAgICAgIF8gICAgXwogfCBfIFx"
                                "fX18gX18oXylfX3wgfF8gIF8gX19fCiB8ICA"
                                "gLyAtX3xfLTwgLyBfYCB8IHx8IC8gLV8pCiB"
                                "8X3xfXF9fXy9fXy9fXF9fLF98XF8sX1xfX198")
              << std::endl;

    printVersion(true);

    Configuration config;
    if (el::Helpers::commandLineArgs()->hasParamWithValue("--residue-home")) {
        config.setHomePath(el::Helpers::commandLineArgs()->getParamValue("--residue-home"));
    }
    config.load(argv[1]);

    if (!config.isValid()) {
        RLOG(ERROR) << "FAILED: There are errors in configuration file" << std::endl << config.errors();
        return 1;
    }

    if (!config.hasFlag(Configuration::Flag::ALLOW_UNMANAGED_LOGGERS)) {
        RVLOG(RV_NOTICE) << "Unmanaged loggers are not be allowed";
    }

    el::LogBuilder* logBuilder = configureLogging(&config);

    CHECK_NOTNULL(logBuilder) << "Unexpected error while configuring Easylogging++";

    try {
        Registry registry(&config);
        CommandHandler commandHandler(&registry);

        std::vector<std::thread> threads;

        // admin server
        threads.push_back(std::thread([&]() {
            el::Helpers::setThreadName("AdminHandler");
            AdminRequestHandler newAdminRequestHandler(&registry, &commandHandler);
            Server svr(config.adminPort(), &newAdminRequestHandler);
            svr.start();
        }));

        // connect server
        threads.push_back(std::thread([&]() {
            el::Helpers::setThreadName("ConnectionHandler");
            ConnectionRequestHandler newConnectionRequestHandler(&registry);
            Server svr(config.connectPort(), &newConnectionRequestHandler);
            svr.start();
        }));

        // log server
        threads.push_back(std::thread([&]() {
            el::Helpers::setThreadName("LogHandler");
            LogRequestHandler logRequestHandler(&registry);
            logRequestHandler.start(); // Start handling incoming requests
            registry.setLogRequestHandler(&logRequestHandler);
            Server svr(config.loggingPort(), &logRequestHandler);
            svr.start();
        }));

        // client integrity task
        threads.push_back(std::thread([&]() {
            el::Helpers::setThreadName("ClientIntegrityTask");
            ClientIntegrityTask task(&registry, registry.configuration()->clientIntegrityTaskInterval());
            registry.setClientIntegrityTask(&task);
            task.start();
        }));

        // log rotator tasks

#define START_LOG_ROTATOR(NAME)\
        threads.push_back(std::thread([&]() {\
            el::Helpers::setThreadName(#NAME);\
            NAME rotator(&registry);\
            registry.addLogRotator(&rotator);\
            rotator.start();\
        }))

        START_LOG_ROTATOR(HourlyLogRotator);
        START_LOG_ROTATOR(SixHoursLogRotator);
        START_LOG_ROTATOR(TwelveHoursLogRotator);
        START_LOG_ROTATOR(DailyLogRotator);
        START_LOG_ROTATOR(WeeklyLogRotator);
        START_LOG_ROTATOR(MonthlyLogRotator);
        START_LOG_ROTATOR(YearlyLogRotator);

#undef START_LOG_ROTATOR

#ifndef RESIDUE_DEV
        // auto updater
        threads.push_back(std::thread([&]() {
            el::Helpers::setThreadName("AutoUpdater");
            AutoUpdater task(&registry, 86400); // run daily
            registry.setAutoUpdater(&task);
            std::string newVer;
            if (task.hasNewVersion(&newVer)) {
                std::cout << "A newer version " << newVer << " is available for download."
                          << " Please visit https://github.com/muflihun/residue/releases/tag/" << newVer
                          << std::endl;
            }
            task.start();
        }));
#else
        // AUTO UPDATER INACTIVE IN DEV MODE
#endif

        if (registry.configuration()->hasFlag(Configuration::Flag::ENABLE_CLI)) {
            signal(SIGINT, interruptHandler); // SIGINT = interrupt
            commandHandler.start(&s_exitOnInterrupt);

            // We aren't accepting any input anymore, detach all the threads
            // to safely end them all
            for (auto& t : threads) {
                t.detach();
            }
        } else {
            for (auto& t : threads) {
                t.join();
            }
        }
    } catch (const std::exception& e) {
        LOG(ERROR) << "Exception: " << e.what();
        return 1;
    }

    return 0;
}
