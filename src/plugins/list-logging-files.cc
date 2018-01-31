//
//  list-logging-files.cc
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

#include "src/plugins/list-logging-files.h"
#include "src/core/registry.h"
#include "src/core/configuration.h"

using namespace residue;

ListLoggingFiles::ListLoggingFiles(Registry* registry) :
    Plugin("files",
              "Lists all the files associated to respective logger or client",
              "files --client-id <id> [--logger-id <id>] [--levels <csv_levels>]",
              registry)
{
}

void ListLoggingFiles::execute(std::vector<std::string>&& params, std::ostringstream& result, bool) const
{
    const std::string clientId = getParamValue(params, "--client-id");
    if (clientId.empty()) {
        result << "No client ID provided" << std::endl;
        return;
    }
    const std::string loggingLevelsStr = getParamValue(params, "--levels");
    std::set<std::string> loggingLevels;
    std::string s;
    std::istringstream loggingLevelsStream(loggingLevelsStr);
    while (std::getline(loggingLevelsStream, s, ',')) {
        loggingLevels.insert(s);
    }
    if (loggingLevels.empty()) {
        loggingLevels.insert("info");
        loggingLevels.insert("warning");
        loggingLevels.insert("error");
        loggingLevels.insert("fatal");
        loggingLevels.insert("debug");
        loggingLevels.insert("trace");
        loggingLevels.insert("verbose");
    }
    const std::string loggerId = getParamValue(params, "--logger-id");

    std::map<std::string, std::set<std::string>> listMap;

    std::string defaultLogger = "default";
    auto foundPos = registry()->configuration()->knownClientDefaultLogger().find(clientId);
    if (foundPos != registry()->configuration()->knownClientDefaultLogger().end()) {
        defaultLogger = foundPos->second;
    }
    listMap.insert(std::make_pair(defaultLogger, std::set<std::string>()));

    if (!loggerId.empty()) {
        if (!registry()->configuration()->isKnownLoggerForClient(clientId, loggerId)) {
            result << "Logger not mapped to client" << std::endl;
            return;
        }
        listMap.insert(std::make_pair(loggerId, std::set<std::string>()));
    } else {
        if (registry()->configuration()->knownClientsLoggers().find(clientId) !=
                registry()->configuration()->knownClientsLoggers().end()) {
            auto setOfLoggerIds = registry()->configuration()->knownClientsLoggers().at(clientId);
            for (std::string loggerId : setOfLoggerIds) {
                listMap.insert(std::make_pair(loggerId, std::set<std::string>()));
            }
        }
    }
    JsonObject::Json jr;
    for (auto& p : listMap) {
        JsonObject::Json j;
        std::string loggerId = p.first;
        j["client_id"] = clientId;
        j["logger_id"] = loggerId;
        for (std::string levelStr : loggingLevels) {
            if (levelStr.empty()) {
                continue;
            }
            el::Level level = el::LevelHelper::convertFromString(levelStr.c_str());
            if (level == el::Level::Unknown) {
                result << "Unknown level [" << levelStr << "]" << std::endl;
                return;
            }
            std::string file = getFile(loggerId, levelStr);
            if (!file.empty()) {
                p.second.insert(file);
            }
        }
        j["files"] = p.second;
        jr.push_back(j);
    }

    result << (jr.dump()) << std::endl;

#ifdef RESIDUE_DEBUG
    RLOG(DEBUG) << "Result: " << result.str();
#endif
}

std::string ListLoggingFiles::getFile(const std::string& loggerId, const std::string& levelStr) const {
    if (!registry()->configuration()->isKnownLogger(loggerId)) {
        return "";
    }
    el::Level level = el::LevelHelper::convertFromString(levelStr.c_str());
    std::string confFile = registry()->configuration()->getConfigurationFile(loggerId);
    // Following line because of weird compiler error of making all the variables
    // inaccessible after confs declaration
    el::Configuration tmpC(el::Level::Info, el::ConfigurationType::Filename, "f");
    (void)tmpC;
    el::Configurations confs(confFile);

    el::Configuration* conf = confs.get(level, el::ConfigurationType::Filename);
    if (conf != nullptr) {
        return conf->value();
    }
    return "";
}
