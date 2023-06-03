//
//  reload-config.cc
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
            result << "Logger ID not provided\n";
        } else {
            reloadLoggerConfig(loggerId, result);
        }
    }

}

void ReloadConfig::reloadServerConfig(std::ostringstream& result) const
{
    Configuration tmpConf(registry()->configuration()->configurationFile());
    if (tmpConf.isValid()) {
        result << "Reloading configurations...\n";
        registry()->reloadConfig();
    } else {
        result << "FAILED to reload configuration. There are errors in configuration file" << std::endl << tmpConf.errors();
    }
}

void ReloadConfig::reloadLoggerConfig(const std::string& loggerId, std::ostringstream& result) const
{
    el::Logger* logger = el::Loggers::getLogger(loggerId, false);
    if (logger == nullptr) {
        result << "Logger [" << loggerId << "] not yet registered\n";
    } else {
        // we need to reload server config as logger config may have updated
        reloadServerConfig(result);
        std::string confFile = registry()->configuration()->getConfigurationFile(loggerId);
        result << "Loading logger configuration...\n";
        el::Configurations config(confFile);
        result << "Reconfiguring logger...\n";
        el::Loggers::reconfigureLogger(logger, config);

        result << "Re-opening the files...\n";
        el::base::type::EnumType lIndex = el::LevelHelper::kMinValid;
        el::LevelHelper::forEachLevel(&lIndex, [&](void) -> bool {
            auto fn = logger->typedConfigurations()->filename(el::LevelHelper::castFromInt(lIndex));
            auto stream = logger->typedConfigurations()->fileStream(el::LevelHelper::castFromInt(lIndex));
            if (stream != nullptr) {
                stream->clear();
                stream->close();
                stream->open(fn, std::ios::out | std::ios::app);
            }
            return false;
        });
        result << "Done!";
    }

}
