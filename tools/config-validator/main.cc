//
//  license-manager-for-test.h
//  Configuration validator
//  Residue
//
//  Copyright 2017-present @abumq (Majid Q.)
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

#include <cstring>
#include <sstream>
#include <iostream>
#include <string>
#include "core/configuration.h"

using namespace residue;

void displayUsage() {
    std::cout << "config-validator <file>" << std::endl;
}

void displayVersion() {
    std::cout << "Config Validator v" << RESIDUE_VERSION << std::endl << std::endl << "Configuration file validator for residue logging server (https://github.com/abumq/residue)" << std::endl;
}

int main(int argc, char* argv[])
{

    if (argc > 1 && strcmp(argv[1], "--version") == 0) {
        displayVersion();
        return 0;
    }

    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        displayUsage();
        return 0;
    }

    bool outputMalformedJson = true;
    std::string configFile;

    for (int i = 0; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg == "--in" && i < argc) {
            configFile = argv[++i];
        } else if (arg == "--no-json" && i < argc) {
            outputMalformedJson = false;
        }
    }

    residue::Configuration config;
    if (!configFile.empty()) {
        config.load(configFile);
    } else {
        std::stringstream ss;
        for (std::string line; std::getline(std::cin, line);) {
            ss << line << std::endl;
        }
        std::string inputJson = ss.str();
        // Remove last 'new line'
        inputJson.erase(inputJson.size() - 1);
        config.loadFromInput(std::move(inputJson));
    }

    if (!config.isValid()) {
        if (config.isMalformedJson() && outputMalformedJson) {
            std::cerr << "Configuration file has errors" << std::endl << config.errors() << std::endl;
        } else if (config.isMalformedJson() && !outputMalformedJson) {
            std::cerr << "Configuration file JSON errors" << std::endl;
        } else {
            // All other errors
            std::cerr << "Configuration file has errors" << std::endl << config.errors() << std::endl;
        }
        return 1;
    }
    std::cout << "Valid file!" << std::endl;
    return 0;
}
