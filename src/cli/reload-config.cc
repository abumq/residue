//
//  reload-config.cc
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

#include "cli/reload-config.h"

#include "core/configuration.h"
#include "core/registry.h"

using namespace residue;

ReloadConfig::ReloadConfig(Registry* registry) :
    Command("rconfig",
            "Reloads specified logger configuration or server configuration",
            "rconfig [--logger-id <logger_id>]",
            registry)
{
}

void ReloadConfig::execute(std::vector<std::string>&& params, std::ostringstream& result, bool) const
{
    if (params.empty()) {
        reloadServerConfig(result);
    } else if (hasParam(params, "--logger-id")) {
        const std::string loggerId = getParamValue(params, "--logger-id");
        if (loggerId.empty()) {
            result << "Logger ID not provided";
        } else {
            reloadLoggerConfig(loggerId, result);
        }
    }

}

void ReloadConfig::reloadServerConfig(std::ostringstream& result) const
{
    Configuration tmpConf(registry()->configuration()->configurationFile());
    if (tmpConf.isValid()) {
        result << "Reloading configurations...";
        registry()->configuration()->reload();
    } else {
        result << "FAILED to reload configuration. There are errors in configuration file" << std::endl << tmpConf.errors();
    }
}

void ReloadConfig::reloadLoggerConfig(const std::string& loggerId, std::ostringstream& result) const
{
    el::Logger* logger = el::Loggers::getLogger(loggerId, false);
    if (logger == nullptr) {
        result << "Logger [" << loggerId << "] not yet registered";
    } else {
        // we need to reload server config as logger config may have updated
        reloadServerConfig(result);
        result << "\n";
        std::string confFile = registry()->configuration()->getConfigurationFile(loggerId);
        result << "Loading logger configuration...\n";
        el::Configurations config(confFile);
        result << "Reconfiguring logger...";
        el::Loggers::reconfigureLogger(logger, config);
        result << "Done!";
    }

}
