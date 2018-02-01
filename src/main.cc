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

#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <functional>
#include <fstream>
#include <memory>
#include <thread>
#include <utility>
#include <boost/asio.hpp>
#include "include/Python.h"
#include "deps/ripe/Ripe.h"
#ifdef RESIDUE_USE_MINE
#   include "deps/mine/mine.h"
#endif
#include "include/log.h"
#include "src/core/configuration.h"
#include "src/crypto/base64.h"
#include "src/core/registry.h"
#include "src/core/command-handler.h"
#include "src/core/residue-exception.h"
#include "src/logging/user-log-builder.h"
#include "src/logging/residue-log-dispatcher.h"
#include "src/logging/log-request-handler.h"
#include "src/logging/known-logger-configurator.h"
#include "src/net/server.h"
#include "src/admin/admin-request-handler.h"
#include "src/connect/connection-request-handler.h"
#include "src/tokenization/token-request-handler.h"
#include "src/tasks/client-integrity-task.h"
#include "src/tasks/auto-updater.h"
#include "src/tasks/log-rotator.h"

using namespace residue;
using boost::asio::ip::tcp;

INITIALIZE_EASYLOGGINGPP

static bool s_exitOnInterrupt = false;

static const std::map<int, std::string> VERBOSE_SEVERITY_MAP
{
    { RV_CRAZY,   "vCRAZY"   },
    { RV_TRACE,   "vTRACE"   },
    { RV_DEBUG,   "vDEBUG"   },
    { RV_DETAILS, "vDETAILS" },
    { RV_5,       "5"        },
    { RV_ERROR,   "vERROR"   },
    { RV_WARNING, "vWARNING" },
    { RV_NOTICE,  "vNOTICE"  },
    { RV_INFO,    "vINFO"    },
    { 0,          ""         }
};

std::string getVerboseSeverityName(const el::LogMessage* message)
{
    return VERBOSE_SEVERITY_MAP.at(message->verboseLevel());
}

el::LogBuilder* configureLogging(Configuration* configuration)
{
    // Configure Easylogging++ to custom settings for residue server

    // Note: Do not add StrictLogFileSizeCheck flag as we manually have log rotator in-place

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
    el::Loggers::getLogger(el::base::consts::kDefaultLoggerId)->setLogBuilder(logBuilder);
    if (std::string(el::base::consts::kDefaultLoggerId) != "default") {
        el::Loggers::getLogger("default")->setLogBuilder(logBuilder);
    }
    el::Loggers::getLogger(el::base::consts::kPerformanceLoggerId)->setLogBuilder(logBuilder);

    // We use our own log dispatcher as we want to do some checks for safety
    el::Helpers::uninstallLogDispatchCallback<el::base::DefaultLogDispatchCallback>("DefaultLogDispatchCallback");
    el::Helpers::installLogDispatchCallback<ResidueLogDispatcher>("ResidueLogDispatcher");
    ResidueLogDispatcher* dispatcher = el::Helpers::logDispatchCallback<ResidueLogDispatcher>("ResidueLogDispatcher");
    dispatcher->setConfiguration(configuration);

    std::string defaultConfigFile = configuration->getConfigurationFile("default");
    el::Configurations defaultLoggerConf = el::Configurations(defaultConfigFile);
    el::Loggers::setDefaultConfigurations(defaultLoggerConf, true);

    // Reset [residue] configuration from residue json configurations
    std::string residueConfigFile = configuration->getConfigurationFile(RESIDUE_LOGGER_ID);
    el::Configurations residueLoggerConf = el::Configurations(residueConfigFile);
    el::Loggers::getLogger(RESIDUE_LOGGER_ID)->configure(residueLoggerConf);

    RVLOG(RV_INFO) << "Default configurations: " << defaultConfigFile;
    RVLOG(RV_INFO) << "Server configurations: " << residueConfigFile;

    // Whenever new logger is registered we will handle it to find right configuration
    // file and configure it accordingly, otherwise we use default configuration
    const std::string configuratorName = "KnownLoggerConfigurator";
    el::Loggers::installLoggerRegistrationCallback<KnownLoggerConfigurator>(configuratorName);
    KnownLoggerConfigurator* configurator = el::Loggers::loggerRegistrationCallback<KnownLoggerConfigurator>(configuratorName);
    configurator->setConfiguration(configuration);
    configurator->setUserLogBuilder(static_cast<const UserLogBuilder*>(logBuilder.get()));
    configurator->setEnabled(true);

    return logBuilder.get();
}

void printVersion(bool addSpaces = false)
{
    std::cout <<  (addSpaces ? "  " : "") << "Residue v" << RESIDUE_VERSION;
#ifdef RESIDUE_SPECIAL_EDITION
    std::cout << "-SE";
#endif
#ifdef RESIDUE_DEBUG
    std::cout << "-debug";
#endif
#ifdef RESIDUE_HAS_EXTENSIONS
    std::cout << " (with extension support) [Python " << RESIDUE_PYTHON_VERSION << "]";
#endif
    std::cout << "\n" << (addSpaces ? "  " : "") << "Built on "
              << __DATE__ << ", " << __TIME__ << std::endl;
}

void interruptHandler(int)
{
    if (s_exitOnInterrupt) {
        el::Helpers::crashAbort(SIGINT);
    } else {
        std::cerr << "(To exit, press ^C again or type quit)" << std::endl;
        s_exitOnInterrupt = true;
    }
}

void generalTerminateHandler(int sig, bool showMsg)
{
    if (showMsg) {
        std::cerr << "Application abnormally terminated." << std::endl;
        std::cerr << "Please report it to us on https://github.com/muflihun/residue/issues" << std::endl;
    }
    el::Helpers::logCrashReason(sig, true, el::Level::Fatal, RESIDUE_LOGGER_ID);
    el::Helpers::crashAbort(sig);
}

void elppCrashHandler(int sig)
{
    generalTerminateHandler(sig, true);
}

void terminateHandler()
{
    generalTerminateHandler(11, false);
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
        std::cerr << "USAGE: residue <residue_config_file>" << std::endl;
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
        std::cout << "Please go to https://github.com/muflihun/residue for help" << std::endl;
        return 0;
    }

    if (!el::Helpers::commandLineArgs()->hasParam("--force-without-root") && el::base::utils::OS::getBashOutput("whoami") != "root") {
        std::cerr << "Please run as 'root'. See https://github.com/muflihun/residue for more details" << std::endl;
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

    Configuration config(argv[1]);

    if (!config.isValid()) {
        RLOG(ERROR) << "FAILED: There are errors in configuration file" << std::endl << config.errors();
        return 1;
    }

    if (!config.hasFlag(Configuration::Flag::ALLOW_UNKNOWN_LOGGERS)) {
        RVLOG(RV_NOTICE) << "Unknown loggers are not be allowed";
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
            boost::asio::io_service io_service;
            AdminRequestHandler newAdminRequestHandler(&registry, &commandHandler);
            Server svr(io_service, config.adminPort(), &newAdminRequestHandler);
            io_service.run();
        }));

        // connect server
        threads.push_back(std::thread([&]() {
            el::Helpers::setThreadName("ConnectionHandler");
            boost::asio::io_service io_service;
            ConnectionRequestHandler newConnectionRequestHandler(&registry);
            Server svr(io_service, config.connectPort(), &newConnectionRequestHandler);
            io_service.run();
        }));

        // log server
        threads.push_back(std::thread([&]() {
            el::Helpers::setThreadName("LogHandler");
            boost::asio::io_service io_service;
            LogRequestHandler logRequestHandler(&registry, logBuilder);
            logRequestHandler.start(); // Start handling incoming requests
            Server svr(io_service, config.loggingPort(), &logRequestHandler);
            io_service.run();
        }));

        // token server
        threads.push_back(std::thread([&]() {
            el::Helpers::setThreadName("TokenHandler");
            boost::asio::io_service io_service;
            TokenRequestHandler tokenRequestHandler(&registry);
            Server svr(io_service, config.tokenPort(), &tokenRequestHandler);
            io_service.run();
        }));

        // client integrity task
        threads.push_back(std::thread([&]() {
            el::Helpers::setThreadName("ClientIntegrityTask");
            ClientIntegrityTask task(&registry, registry.configuration()->clientIntegrityTaskInterval());
            registry.setClientIntegrityTask(&task);
            task.start();
        }));

        // log rotator task
        threads.push_back(std::thread([&]() {
            el::Helpers::setThreadName("LogRotator");
            LogRotator rotator(&registry, Configuration::RotationFrequency::HOURLY); // lowest denominator
            registry.setLogRotator(&rotator);
            rotator.start();
        }));

        // auto updater
        threads.push_back(std::thread([&]() {
            el::Helpers::setThreadName("AutoUpdater");
            AutoUpdater task(&registry, 86400); // run daily
            registry.setAutoUpdater(&task);
            std::string newVer;
            if (task.check(&newVer)) {
                std::cout << "A newer version " << newVer
                          << " is available for download. Please visit https://muflihun.github.io/residue/";
            }
            task.start();
        }));

        if (registry.configuration()->hasFlag(Configuration::Flag::ACCEPT_INPUT)) {
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
