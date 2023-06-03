//
//  list-logging-files.cc
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

#include "cli/list-logging-files.h"

#include "core/configuration.h"
#include "core/json-builder.h"
#include "core/registry.h"

using namespace residue;

ListLoggingFiles::ListLoggingFiles(Registry* registry) :
    Command("files",
            "Lists all the files associated to respective logger or client",
            "files --client-id <id> [--logger-id <id>] [--levels <csv_levels>]",
            registry)
{
}

void ListLoggingFiles::execute(std::vector<std::string>&& params, std::ostringstream& result, bool) const
{
    const std::string clientId = getParamValue(params, "--client-id");
    if (clientId.empty()) {
        result << "No client ID provided";
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

    if (!loggerId.empty()) {
        if (!registry()->configuration()->isManagedLoggerForClient(clientId, loggerId)) {
            result << "Logger not mapped to client";
            return;
        }
        listMap.insert(std::make_pair(loggerId, std::set<std::string>()));
    } else {
        if (registry()->configuration()->managedClientsLoggers().find(clientId) !=
                registry()->configuration()->managedClientsLoggers().end()) {
            auto setOfLoggerIds = registry()->configuration()->managedClientsLoggers().at(clientId);
            for (std::string loggerId : setOfLoggerIds) {
                listMap.insert(std::make_pair(loggerId, std::set<std::string>()));
            }
        }
    }
    const std::size_t capacity = 2048;
    char source[capacity];
    JsonBuilder j(source, capacity);
    DRVLOG(RV_DEBUG_2) << "Starting JSON serialization with [" << capacity << "] bytes";
    j.startArray();
    for (auto& p : listMap) {
        j.startObject();
        std::string loggerId = p.first;
        j.addValue("client_id", clientId);
        j.addValue("logger_id", loggerId);
        std::set<std::string> uniqFiles;
        j.startArray("files");
        for (std::string levelStr : loggingLevels) {
            if (levelStr.empty()) {
                continue;
            }
            el::Level level = el::LevelHelper::convertFromString(levelStr.c_str());
            if (level == el::Level::Unknown) {
                result << "Unknown level [" << levelStr << "]";
                return;
            }
            std::string file = getFile(loggerId, levelStr);
            if (!file.empty()) {
                uniqFiles.insert(file);
            }
        }
        for (std::string file : uniqFiles) {
            j.addValue(file);
        }
        j.endArray(); // files
        j.endObject();
    }
    j.endArray();
    std::string dump = source;
    result << (dump);

 #ifdef RESIDUE_DEBUG
    RLOG(DEBUG) << "Result: " << result.str();
 #endif
}

std::string ListLoggingFiles::getFile(const std::string& loggerId, const std::string& levelStr) const {
    if (!registry()->configuration()->isManagedLogger(loggerId)) {
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
