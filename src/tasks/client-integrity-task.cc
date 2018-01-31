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

#include <vector>
#include "include/log.h"
#include "src/tasks/client-integrity-task.h"
#include "src/core/registry.h"
#include "src/clients/client.h"
#include "src/tokenization/token.h"

using namespace residue;

ClientIntegrityTask::ClientIntegrityTask(Registry* registry,
                                         unsigned int interval) :
    Task("ClientIntegrityTask", registry, interval)
{
}

void ClientIntegrityTask::execute()
{
    auto* list = &(m_registry->clients());
    for (auto clientIter = list->begin(); clientIter != list->end();) {
        Client* client = &(clientIter->second);
        if (!client->isAlive()) {
            RVLOG(RV_WARNING) << "Client expired " << client->id();
            // Do not use m_registry.removeClient, instead,
            // manually remove as we need to manually increment
            // iterator as erase invalidates it
            std::lock_guard<std::recursive_mutex> lock(m_registry->mutex());
            m_registry->clients().erase(clientIter++);
        } else {
            ++clientIter;
        }
    }
}
