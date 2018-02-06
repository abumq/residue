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

#include "src/cli/reload-config.h"
#include "src/core/registry.h"
#include "src/core/configuration.h"

using namespace residue;

ReloadConfig::ReloadConfig(Registry* registry) :
    Command("reload",
            "Reloads configuration",
            "reload",
            registry)
{
}

void ReloadConfig::execute(std::vector<std::string>&&, std::ostringstream& result, bool) const
{
    Configuration tmpConf(registry()->configuration()->configurationFile());
    if (tmpConf.isValid()) {
        result << "Reloading configurations...";
        registry()->configuration()->reload();
    } else {
        result << "FAILED to reload configuration. There are errors in configuration file" << std::endl << tmpConf.errors();
    }
}
