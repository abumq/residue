//
//  client-integrity-task.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
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