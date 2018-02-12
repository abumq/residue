//
//  stats.cc
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

#include "cli/stats.h"
#include "core/registry.h"
#include "clients/client.h"
#include "utils/utils.h"

using namespace residue;

Stats::Stats(Registry* registry) :
    Command("stats",
            "Displays current session details e.g, active sessions etc",
            "stats [list] [--client-id <client_id>]",
            registry)
{
}

void Stats::execute(std::vector<std::string>&& params, std::ostringstream& result, bool) const
{
    if (params.empty()) {
        result << "Recv: " << registry()->bytesReceived() << "b, Sent: "
               << registry()->bytesSent() << "b"
               << std::endl;
        return;
    }

    if (hasParam(params, "list")) {
        std::string clientId = getParamValue(params, "--client-id");
        std::ostringstream tmpR;
        std::size_t i = 1;
        auto now = Utils::now();
        for (auto& activeSession : registry()->activeSessions()) {
            if (!clientId.empty()) {
                if (activeSession.session->client() == nullptr
                        || activeSession.session->client()->id() != clientId) {
                    continue;
                }
            }
            tmpR << (i++)  << " " << "> ID: " << activeSession.session->id();
            if (activeSession.session->client() != nullptr) {
                tmpR << ", Client: " << activeSession.session->client()->id();
            }
            tmpR << ", Active for " << (now - activeSession.timeCreated) << "s"
                   << ", Sent: " << activeSession.session->bytesSent() << "b"
                   << ", Recv: " << activeSession.session->bytesReceived() << "b";
            tmpR << std::endl;
        }
        result << "Active sessions";
        if (!clientId.empty()) {
            result << " by [" << clientId << "]: " << (i - 1) << std::endl;
        } else {
            result << ": " << registry()->activeSessions().size() << std::endl;
        }
        result << tmpR.str();
    }
}
