//
//  client-integrity-task.cc
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

#include "tasks/client-integrity-task.h"

#include <vector>

#include "core/client.h"
#include "core/configuration.h"
#include "core/registry.h"
#include "logging/log.h"

using namespace residue;

ClientIntegrityTask::ClientIntegrityTask(Registry* registry,
                                         unsigned int interval) :
    Task("ClientIntegrityTask", registry, interval)
{
}

void ClientIntegrityTask::performCleanup(const std::string& clientId)
{
    auto iter = m_registry->clients().find(clientId);
    if (iter == m_registry->clients().end()) {
        // This is possible in case of "unknown"
        return;
    }
    Client* client = &(iter->second);
    if (!client->isAlive()) {
        // Do not use m_registry.removeClient, instead,
        // manually remove as we need to manually increment
        // iterator as erase invalidates it
        std::lock_guard<std::recursive_mutex> lock(m_registry->mutex());
        m_registry->clients().erase(iter);
    } else {
        if (!client->backupKey().empty()) {
            RVLOG(RV_WARNING) << "Removing backup key for [" << client->id() << "]";
            client->setBackupKey("");
        }
    }
}

void ClientIntegrityTask::execute()
{
    auto* list = &(m_registry->clients());
    for (auto clientIter = list->begin(); clientIter != list->end();) {
        Client* client = &(clientIter->second);
        if (!client->isAlive()) {

            std::lock_guard<std::mutex> pausedClientsLock(m_mutex);
            auto pos = m_pausedClients.find(client->id());
            if (pos == m_pausedClients.end()) {

                // here we check for unknown clients
                // we may be removing a "paused" unknown client
                // as m_pausedClients for unknown client will be "unknown"
                if (!client->isKnown() && m_pausedClients.find(Configuration::UNKNOWN_CLIENT_ID) != m_pausedClients.end()) {
                    // yes we are surely removing a paused unknown client
                    // that we shouldn't do
                    RLOG(INFO) << "Unknown client [" << client->id() << "] expired - Paused removal";
                    ++clientIter; // skip and move on
                } else {
                    RLOG(INFO) << "Client [" << client->id() << "] expired";
                    // Do not use m_registry.removeClient, instead,
                    // manually remove as we need to manually increment
                    // iterator as erase invalidates it
                    std::lock_guard<std::recursive_mutex> lock(m_registry->mutex());
                    m_registry->clients().erase(clientIter++);
                }
            } else {
                RLOG(INFO) << "Client [" << client->id() << "] expired - Paused removal";
                ++clientIter; // skip and move on
            }
        } else {
            if (!client->backupKey().empty()) {
                RVLOG(RV_WARNING) << "Removing backup key for [" << client->id() << "]";
                client->setBackupKey("");
            }
            ++clientIter;
        }
    }

    if (!m_registry->configuration()->hasFlag(Configuration::Flag::REQUIRES_TIMESTAMP)) {
        RLOG(WARNING) << "You have disabled 'requires_timestamp'. Your server is prone to replay attack.";
    }
}
