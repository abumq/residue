//
//  registry.h
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

#ifndef Registry_h
#define Registry_h

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "src/non-copyable.h"
#include "src/clients/client.h"
#include "src/cli/command-handler.h"
#include "src/net/session.h"
#include "src/utils/utils.h"

namespace residue {

class Configuration;
class ClientIntegrityTask;
class AutoUpdater;
class LogRotator;

///
/// \brief Registry for client with helper functions
///
class Registry final : NonCopyable
{
public:

    explicit Registry(Configuration* configuration);

    bool addClient(const Client& client);
    bool updateClient(const Client& client);
    Client* findClient(const std::string& clientId);
    void removeClient(Client* client);

    inline Configuration* configuration() const
    {
        return m_configuration;
    }

    inline std::unordered_map<std::string, Client>& clients()
    {
        return m_clients;
    }

    inline std::unordered_map<std::shared_ptr<Session>, types::Time>& activeSessions()
    {
        return m_activeSessions;
    }

    inline bool clientExists(const std::string& clientId) const
    {
        return m_clients.find(clientId) != m_clients.end();
    }

    inline void join(std::shared_ptr<Session>&& session)
    {
        std::lock_guard<std::recursive_mutex> lock_(m_sessMutex);
        m_activeSessions.insert(std::make_pair(session, Utils::now()));
    }

    inline void leave(std::shared_ptr<Session>&& session)
    {
        std::lock_guard<std::recursive_mutex> lock_(m_sessMutex);
        m_activeSessions.erase(session);
    }

    inline const std::string& bytesReceived() const
    {
        return m_bytesReceived;
    }

    inline const std::string& bytesSent() const
    {
        return m_bytesSent;
    }

    inline void addBytesReceived(const std::size_t& v)
    {
        Utils::bigAdd(m_bytesReceived, std::to_string(v));
    }

    inline void addBytesSent(const std::size_t& v)
    {
        Utils::bigAdd(m_bytesSent, std::to_string(v));
    }

    inline std::recursive_mutex& mutex()
    {
        return m_mutex;
    }

    inline std::vector<LogRotator*>& logRotators()
    {
        return m_logRotators;
    }

    inline void addLogRotator(LogRotator* logRotator)
    {
        if (std::find(m_logRotators.begin(), m_logRotators.end(), logRotator) == m_logRotators.end()) {
            m_logRotators.push_back(logRotator);
        }
    }

    inline ClientIntegrityTask* clientIntegrityTask()
    {
        return m_clientIntegrityTask;
    }

    inline void setClientIntegrityTask(ClientIntegrityTask* clientIntegrityTask)
    {
        m_clientIntegrityTask = clientIntegrityTask;
    }

    inline AutoUpdater* autoUpdater()
    {
        return m_autoUpdater;
    }

    inline void setAutoUpdater(AutoUpdater* autoUpdater)
    {
        m_autoUpdater = autoUpdater;
    }

    void reset();
private:
    friend class CommandHandler;

    Configuration* m_configuration;
    std::vector<LogRotator*> m_logRotators;
    ClientIntegrityTask* m_clientIntegrityTask;
    AutoUpdater* m_autoUpdater;

    std::unordered_map<std::string, Client> m_clients;
    std::unordered_map<std::shared_ptr<Session>, types::Time> m_activeSessions;

    std::recursive_mutex m_mutex;
    std::recursive_mutex m_sessMutex;

    std::string m_bytesSent;
    std::string m_bytesReceived;

};
}
#endif /* Registry_h */
