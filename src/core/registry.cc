//
//  registry.cc
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

#include <utility>
#include <algorithm>
#include "logging/log.h"
#include "core/registry.h"
#include "utils/utils.h"
#include "core/configuration.h"

using namespace residue;

Registry::Registry(Configuration* configuration) :
    m_configuration(configuration),
    m_clientIntegrityTask(nullptr),
    m_autoUpdater(nullptr),
    m_bytesSent("0"),
    m_bytesReceived("0")
{
}

bool Registry::addClient(const Client& client)
{
    DRVLOG(RV_DEBUG) << "Attempting to add client @" << this;
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if (clientExists(client.id())) {
        return false;
    }
    DRVLOG(RV_DEBUG) << "Adding client @" << this;
    m_clients.insert(std::make_pair(client.id(), client));
    return true;
}

bool Registry::updateClient(const Client& client)
{
    DRVLOG(RV_DEBUG) << "Attempting to update client @" << this;
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    const auto& it = m_clients.find(client.id());
    if (it == m_clients.end()) {
        return false;
    }
    DRVLOG(RV_DEBUG) << "Updating client @" << this;
    Client* existingClient = &(it->second);
    existingClient->setKey(client.key());
    existingClient->setAge(client.age());
    existingClient->setDateCreated(client.dateCreated());
    existingClient->setKeySize(client.keySize());
    existingClient->setAcknowledged(client.acknowledged());
    existingClient->setRsaPublicKey(client.rsaPublicKey());
    return true;
}

Client* Registry::findClient(const std::string& clientId)
{
    const auto& iter = m_clients.find(clientId);
    if (iter == m_clients.end()) {
        return nullptr;
    }
    return &iter->second;
}

void Registry::removeClient(Client* client)
{
    DRVLOG(RV_DEBUG) << "Removing client @" << this;
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if (client != nullptr) {
        m_clients.erase(client->id());
    }
}

void Registry::reset()
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    RLOG(INFO) << "Reloading configurations...";
    m_configuration->reload();
    RLOG(INFO) << "Resetting clients...";
    m_clients.clear();
    RLOG(INFO) << "Resetting sessions...";
    m_activeSessions.clear();
}
