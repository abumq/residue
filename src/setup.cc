//
//  setup.cc
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

#include "setup.h"

#include <fstream>

#include "ripe/Ripe.h"

#include "core/configuration.h"
#include "extensions/log-extension.h"
#include "extensions/pre-archive-extension.h"
#include "extensions/post-archive-extension.h"
#include "extensions/dispatch-error-extension.h"
#include "logging/log.h"
#include "utils/utils.h"

using namespace residue;

const std::string Setup::kDefaultLoggerConf = std::string(R"(* GLOBAL:
  FORMAT                  =   "%datetime [%logger] %level %msg"
  FILENAME                =   "/var/log/residue/default.log"
  ENABLED                 =   true
  TO_FILE                 =   true
  SUBSECOND_PRECISION      =   3
  PERFORMANCE_TRACKING    =   false
* VERBOSE:
  FORMAT                  =   "%datetime [%logger] %level-%vlevel %msg")");

const std::string Setup::kResidueLoggerConf = std::string(R"(* GLOBAL:
  FORMAT                  =   "%datetime [%logger] %level %msg"
  FILENAME                =   "/var/log/residue/residue.log"
  ENABLED                 =   true
  TO_FILE                 =   true
  SUBSECOND_PRECISION      =   3
  PERFORMANCE_TRACKING    =   false
* VERBOSE:
  FORMAT                  =   "%datetime [%logger] %level-%vlevel %msg")");

template <typename T>
class ExtensionTemplateForSetup : public T {
public:
    explicit ExtensionTemplateForSetup(const std::string& id) : T(id)
    {
    }

    virtual Extension::Result execute(const typename T::Data* const) override
    {
        return { 0, true};
    }
};

int Setup::setup()
{
    if (el::Helpers::commandLineArgs()->hasParamWithValue("--help")
            || !el::Helpers::commandLineArgs()->hasParamWithValue("--output-dir")) {
        std::cout << "USAGE: residue --setup --output-dir=<directory> [--clients=<3>]" << std::endl;
        return 1;
    }

    int noOfClients = 3;
    if (el::Helpers::commandLineArgs()->hasParamWithValue("--clients")) {
        noOfClients = atoi(el::Helpers::commandLineArgs()->getParamValue("--clients"));
    }

    RLOG(INFO) << "Configuring residue for " << noOfClients << " clients.";

    Configuration config;
    if (el::Helpers::commandLineArgs()->hasParamWithValue("--residue-home")) {
        config.setHomePath(el::Helpers::commandLineArgs()->getParamValue("--residue-home"));
    }

    std::string outputDir = el::Helpers::commandLineArgs()->getParamValue("--output-dir");

    if (outputDir.empty()) {
        std::cerr << "Output directory must be provided";
        return 1;
    }

    if (outputDir.at(outputDir.size() - 1) != '/') {
        outputDir.append("/");
    }

    const std::string keysPath = outputDir + "/keys/";
    RLOG(INFO) << ">> Creating " << keysPath;
    if (!Utils::createPath(keysPath)) {
        RLOG(ERROR) << "Path [" << outputDir << "] is not writable";
        return 1;
    }

    const std::string loggerConfigPath = outputDir + "/loggers/";
    RLOG(INFO) << ">> Creating " << loggerConfigPath;
    Utils::createPath(loggerConfigPath);

    std::fstream fs;
    fs.open(loggerConfigPath + "residue.conf", std::fstream::out);
    fs << kResidueLoggerConf;
    fs.flush();
    fs.close();

    RLOG(INFO) << ">> Generating configuration....";

    // generate server key
    config.m_serverKey = Ripe::generateNewKey(32);
    std::string keySuffix = keysPath + "server";
    RLOG(INFO) << ">> Generating server keypair...";
    Ripe::writeRSAKeyPair(keySuffix + ".pub", keySuffix + ".key", 2048);
    config.m_serverRSAPublicKeyFile = "$RESIDUE_HOME/keys/server.pub";
    config.m_serverRSAPrivateKeyFile = "$RESIDUE_HOME/keys/server.key";

    // generate known clients
    Utils::createPath(keysPath + "/clients/");
    for (int i = 1; i <= noOfClients; ++i) {
        std::string clientId = std::string("client") + std::to_string(i);

        RLOG(INFO) << ">> Generating " << clientId;

        keySuffix = keysPath + "clients/" + clientId;
        RLOG(INFO) << ">> Generating keypair...";
        Ripe::writeRSAKeyPair(keySuffix + ".pub", keySuffix + ".key", 2048);

        fs.open(keySuffix + ".pub", std::fstream::in);
        std::string publicKeyContents = std::string(std::istreambuf_iterator<char>(fs),
                                                    std::istreambuf_iterator<char>());
        fs.close();

        config.m_managedClientsKeys.insert(std::make_pair(clientId,
                                                        std::make_pair("$RESIDUE_HOME/keys/clients/" + clientId + ".pub", publicKeyContents)));
    }

    RLOG(INFO) << ">> Generating logger config...";
    fs.open(loggerConfigPath + "default.logger.conf", std::fstream::out);
    fs << kDefaultLoggerConf;
    fs.flush();
    fs.close();

    // generate known loggers
    config.m_configurations.insert(std::make_pair("residue", "$RESIDUE_HOME/loggers/residue.logger.conf"));
    config.m_configurations.insert(std::make_pair("default", "$RESIDUE_HOME/loggers/default.logger.conf"));

    config.m_rotationFrequencies.insert(std::make_pair("residue", Configuration::DAILY));
    config.m_rotationFrequencies.insert(std::make_pair("default", Configuration::DAILY));

    config.m_adminPort = 8776;
    config.m_connectPort = 8777;
    config.m_loggingPort = 8778;
    config.m_fileMode = static_cast<unsigned int>(S_IRUSR | S_IWUSR | S_IRGRP);
    config.m_defaultKeySize = 256;
    config.addFlag(Configuration::ENABLE_CLI);
    config.addFlag(Configuration::ALLOW_INSECURE_CONNECTION);
    config.addFlag(Configuration::ALLOW_UNMANAGED_LOGGERS);
    config.addFlag(Configuration::ALLOW_UNMANAGED_CLIENTS);
    config.addFlag(Configuration::REQUIRES_TIMESTAMP);
    config.addFlag(Configuration::COMPRESSION);
    config.addFlag(Configuration::ALLOW_BULK_LOG_REQUEST);
    config.m_maxItemsInBulk = 50;
    config.m_timestampValidity = 120;
    config.m_nonAcknowledgedClientAge = 300;
    config.m_clientIntegrityTaskInterval = 300;
    config.m_clientAge = 3600;
    config.m_dispatchDelay = 1;

    config.m_archivedLogDirectory = "%original/archives/";
    config.m_archivedLogCompressedFilename = "%logger.%wday.tar.gz";
    config.m_archivedLogFilename = "%logger.%wday.log";

#if 0
    // log extensions @ setup time
    ExtensionTemplateForSetup<LogExtension> e("test");
    e.m_modulePath = "the/path/so.so";
    e.m_description = "desc";
    e.m_config.parse("{\"id\":2}");
    config.m_logExtensions.push_back(&e);
#endif
    RLOG(INFO) << ">> Writing " << outputDir << "residue.json.conf";

    std::fstream fconfig(outputDir + "residue.json.conf", std::ios::out);
    fconfig << config.exportAsString() << std::endl;
    fconfig.close();

    RLOG(INFO) << ">> Now set your RESIDUE_HOME to " << outputDir
               << " (via environment variable or using --residue-home command line argument)";

    return 0;
}
