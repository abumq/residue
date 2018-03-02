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

#include "cli/command-handler.h"
#include "core/client.h"
#include "net/session.h"
#include "non-copyable.h"
#include "utils/utils.h"

namespace residue {

class Configuration;
class ClientIntegrityTask;
class AutoUpdater;
class LogRotator;
class LogRequestHandler;

///
/// \brief Registry for client with helper functions
///
class Registry final : NonCopyable
{
public:

    struct ActiveSession
    {
        std::shared_ptr<Session> session;
        types::Time timeCreated;
    };

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

    inline std::vector<ActiveSession>& activeSessions()
    {
        return m_activeSessions;
    }

    inline bool clientExists(const std::string& clientId) const
    {
        return m_clients.find(clientId) != m_clients.end();
    }

    void join(const std::shared_ptr<Session>& session);

    void leave(const std::shared_ptr<Session>& session);

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

    inline LogRequestHandler* logRequestHandler()
    {
        return m_logRequestHandler;
    }

    inline void setLogRequestHandler(LogRequestHandler* logRequestHandler)
    {
        m_logRequestHandler = logRequestHandler;
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
    void reloadConfig();
private:
    friend class CommandHandler;

    Configuration* m_configuration;

    std::vector<LogRotator*> m_logRotators;
    std::vector<ActiveSession> m_activeSessions;
    ClientIntegrityTask* m_clientIntegrityTask;
    LogRequestHandler* m_logRequestHandler;
    AutoUpdater* m_autoUpdater;

    std::unordered_map<std::string, Client> m_clients;

    std::recursive_mutex m_mutex;
    std::recursive_mutex m_sessMutex;

    std::string m_bytesSent;
    std::string m_bytesReceived;

};
}
#endif /* Registry_h */
