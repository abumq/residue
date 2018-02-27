//
//  update.cc
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

#include "cli/update.h"

#include "core/registry.h"
#include "tasks/auto-updater.h"

using namespace residue;

Update::Update(Registry* registry) :
    Command("update",
            "Check for updates and update the server.",
            "update [--check-only]",
            registry)
{
}

void Update::execute(std::vector<std::string>&& params, std::ostringstream& result, bool) const
{
    if (params.empty() || params.at(0) == "--check-only") {
        std::string newVer;
        if (registry()->autoUpdater()->hasNewVersion(&newVer)) {
            result << "You are running v" << RESIDUE_VERSION << ", new version is available: " << newVer << std::endl;
        } else {
            result << "You are running latest version v" << RESIDUE_VERSION;
        }
    }
}
