//
//  list-tokens.cc
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

#include "src/plugins/list-tokens.h"
#include "src/core/registry.h"

using namespace residue;

ListTokens::ListTokens(Registry* registry) :
    Plugin("tokens",
              "List all the tokens and their respective status for client",
              "tokens --client-id <id>",
              registry)
{
}

void ListTokens::execute(std::vector<std::string>&& params, std::ostringstream& result, bool) const
{
    const std::string clientId = getParamValue(params, "--client-id");
    if (clientId.empty()) {
        result << "\nNo client ID provided" << std::endl;
        return;
    }
    for (auto& c : registry()->clients()) {
        if (c.second.id() == clientId) {
            for (auto& l: c.second.tokens()) {
                result << "Logger: " << l.first << std::endl;
                int i = 1;
                for (auto& t: l.second) {
                    result << (i++) << " > " << t.data() << " Age: " << t.age() << "s " << (t.isValid() ? "" : "(EXPIRED)") << std::endl;
                }
            }
        }
    }
}
