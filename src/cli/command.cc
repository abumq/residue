//
//  command.cc
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

#include "cli/command.h"

#include "core/registry.h"

using namespace residue;

Command::Command(const std::string& name,
                 const std::string& description,
                 const std::string& help,
                 Registry* registry) :
    m_name(name),
    m_description(description),
    m_help(help),
    m_registry(registry)
{
}

bool Command::getConfirmation(const std::string& help) const
{
    std::cout << "Are you sure? (Y/N" << (help.empty() ? "" : "/?") << "): ";
    std::string confirm;
    std::getline(std::cin, confirm);
    if (Utils::toLower(confirm) == "y" || Utils::toLower(confirm) == "yes") {
        return true;
    } else if (confirm == "?") {
        std::cout << help << "\n";
    }
    std::cout << std::endl;
    return false;
}

std::string Command::getParamValue(const std::vector<std::string>& params,
                                   const std::string& param) const
{
    auto pos = std::find(params.begin(), params.end(), param);
    if (pos != params.end()) {
        if (pos < params.end() - 1) {
            auto nx = std::next(pos);
            return *nx;
        }
    }
    return "";
}
