//
//  reset.cc
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

#include "cli/reset.h"

#include "core/registry.h"

using namespace residue;

Reset::Reset(Registry* registry) :
    Command("reset",
            "Resets the server to default state",
            "reset",
            registry)
{
}

void Reset::execute(std::vector<std::string>&&, std::ostringstream& result, bool ignoreConfirmation) const
{
    if (ignoreConfirmation || getConfirmation("Reset does following things to the server:\n"
                                              "1. Reloads configurations\n"
                                              "2. Disconnect all the clients\n"
                                              "3. Disconnect active sessions\n")) {
        registry()->reset();
        result << "Reset successful";
    }
}
