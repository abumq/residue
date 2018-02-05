//
//  session-details.cc
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

#include "src/cli/session-details.h"
#include "src/core/registry.h"
#include "src/utils/utils.h"

using namespace residue;

SessionDetails::SessionDetails(Registry* registry) :
    Command("sess",
            "Displays current session details e.g, active sessions etc",
            "sess [--stats]",
            registry)
{
}

void SessionDetails::execute(std::vector<std::string>&& params, std::ostringstream& result, bool) const
{
    result << "Active sessions: " << registry()->activeSessions().size() << std::endl;
    if (hasParam(params, "--stats")) {
        int i = 1;
        auto now = Utils::now();
        for (auto& session : registry()->activeSessions()) {
            result << (i++) << " > Recv: " << session.first->bytesReceived()
                   << ", Sent: " << session.first->bytesSent()
                   << ", Active for " << (now - session.second) << " s" << std::endl;
        }
    }
}
