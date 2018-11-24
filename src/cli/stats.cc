//
//  stats.cc
//  Residue
//
//  Copyright 2017-present Zuhd Web Services
//  https://zuhd.org
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

#include <iomanip>

#include "core/client.h"
#include "core/registry.h"
#include "logging/log-request-handler.h"
#include "logging/residue-log-dispatcher.h"
#include "utils/utils.h"

using namespace residue;

Stats::Stats(Registry* registry) :
    Command("stats",
            "Displays current session details e.g, active sessions, queue and buffer info etc",
            "stats [list] [dyn] [queue] [--client-id <client_id>]",
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
    } else if (hasParam(params, "dyn")) {
        // check dynamic buffer
        ResidueLogDispatcher* dispatcher = el::Helpers::logDispatchCallback<ResidueLogDispatcher>("ResidueLogDispatcher");
        if (dispatcher != nullptr) {
            if (dispatcher->m_dynamicBuffer.empty()) {
                result << "Dynamic buffer is empty";
            } else {
                result << "Dynamic buffer information:\n";
                for (auto& pair : dispatcher->m_dynamicBuffer) {
                    result << "Logger: " << pair.second.logger->id() << "\t";
                    result << "Filename: " << pair.first << "\t";
                    result << "Items: " << pair.second.lines.size() << "\n";
                }
            }
        } else {
            result << "Could not extract dispatcher";
        }
    } else if (hasParam(params, "queue")) {
        auto displayQueueStat = [&](const std::string& clientId) {
            auto pos = registry()->logRequestHandler()->m_queueProcessor.find(clientId);
            if (pos == registry()->logRequestHandler()->m_queueProcessor.end()) {
                result << "ERR: Client not registered in processor";
            } else {
                const ClientQueueProcessor* processor = registry()->logRequestHandler()->m_queueProcessor.at(clientId).get();
                result << "Queue For: " << std::setw(20) << std::left << clientId << " ";
                result << "Active:" << std::setw(6) << std::right << processor->m_queue.size() << " ";
                result << "Backlog:" << std::setw(6) << std::right << processor->m_queue.backlogSize();
                if (hasParam(params, "sampling")) {
                    std::string sampleCount = getParamValue(params, "sampling");
                    if (sampleCount.empty()) {
                        sampleCount = "3";
                    }
                    int sc = atoi(sampleCount.c_str());
                    if (sc < 1 || sc > 10) {
                        sc = 3;
                    }
                    auto size = processor->m_queue.size();
                    if (size > 0) {
                        std::this_thread::sleep_for(std::chrono::seconds(sc));
                        auto newSize = processor->m_queue.size();
                        result << " Speed:" << std::setw(3) << std::right << (size - newSize) / sc << " items/s (incl. bulk)";
                    }
                }
                result << "\n";
            }
        };
        std::string clientId = getParamValue(params, "--client-id");
        if (clientId.empty()) {
            for (auto& pair : registry()->logRequestHandler()->m_queueProcessor) {
                clientId = pair.first;
                displayQueueStat(clientId);
            }
        } else {
            displayQueueStat(clientId);
        }
    }
}
