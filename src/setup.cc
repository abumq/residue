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

#include <fstream>
#include <string>

#include "ripe/Ripe.h"

#include "core/configuration.h"
#include "logging/log.h"
#include "utils/utils.h"

using namespace residue;

const std::string kDefaultLoggerConf = std::string(R"(* GLOBAL:
  FORMAT                  =   "%datetime [%logger] %level %msg"
  FILENAME                =   "/var/log/residue/default.log"
  ENABLED                 =   true
  TO_FILE                 =   true
  SUBSECOND_PRECISION      =   3
  PERFORMANCE_TRACKING    =   false
* VERBOSE:
  FORMAT                  =   "%datetime [%logger] %level-%vlevel %msg")");

const std::string kResidueLoggerConf = std::string(R"(* GLOBAL:
  FORMAT                  =   "%datetime [%logger] %level %msg"
  FILENAME                =   "/var/log/residue/residue.log"
  ENABLED                 =   true
  TO_FILE                 =   true
  SUBSECOND_PRECISION      =   3
  PERFORMANCE_TRACKING    =   false
* VERBOSE:
  FORMAT                  =   "%datetime [%logger] %level-%vlevel %msg")");

int setup()
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
    Utils::createPath(keysPath);

    const std::string loggerConfigPath = outputDir + "/loggers/";
    RLOG(INFO) << ">> Creating " << loggerConfigPath;
    Utils::createPath(loggerConfigPath);


    std::fstream fs;
    fs.open(loggerConfigPath + "residue.conf", std::fstream::out);
    fs << kResidueLoggerConf;
    fs.flush();
    fs.close();

    for (int i = 1; i <= noOfClients; ++i) {

        RLOG(INFO) << ">> Generating client " << i;

        std::string keySuffix = keysPath + "client" + std::to_string(i);
        RLOG(INFO) << ">> Generating keypair...";
        Ripe::writeRSAKeyPair(keySuffix + ".pub", keySuffix + ".key", 2048);

        RLOG(INFO) << ">> Generating logger config...";
        fs.open(loggerConfigPath + "client" + std::to_string(i) + ".logger.conf", std::fstream::out);
        fs << kDefaultLoggerConf;
        fs.flush();
        fs.close();
    }

    // add known loggers


    RLOG(INFO) << ">> Writing " << outputDir << "residue.json.conf";

    std::fstream fconfig(outputDir + "residue.json.conf", std::ios::out);
    fconfig << config.exportAsString() << std::endl;
    fconfig.close();

    return 0;
}
