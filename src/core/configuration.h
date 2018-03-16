//
//  configuration.h
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

#ifndef Configuration_h
#define Configuration_h

#include <algorithm>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/client.h"
#include "core/json-doc.h"
#include "crypto/rsa.h"
#include "non-copyable.h"

namespace residue {

class AdminRequestHandler;
class UserLogBuilder;
class LogRequest;
class Extension;

///
/// \brief Residue server configuration
///
class Configuration final : NonCopyable
{
public:

    enum Flag : unsigned int
    {
        NONE = 0,
        ALLOW_UNMANAGED_LOGGERS = 1,
        ALLOW_BULK_LOG_REQUEST = 16,
        IMMEDIATE_FLUSH = 32,
        ALLOW_UNMANAGED_CLIENTS = 64,
        ALLOW_INSECURE_CONNECTION = 128,
        COMPRESSION = 256,
        ENABLE_CLI = 512,
        REQUIRES_TIMESTAMP = 1024,
        ENABLE_DYNAMIC_BUFFER = 2048,
    };

    enum RotationFrequency : types::Time
    {
        NEVER = 0,
        HOURLY = 60 * 60,
        SIX_HOURS = RotationFrequency::HOURLY * 6,
        TWELVE_HOURS = RotationFrequency::HOURLY * 12,
        DAILY = RotationFrequency::HOURLY * 24,
        WEEKLY = RotationFrequency::DAILY * 7,
        MONTHLY = RotationFrequency::WEEKLY * 4,
        YEARLY = RotationFrequency::MONTHLY * 12
    };

    ///
    /// \brief For processor thread ID
    ///
    static const std::string UNMANAGED_CLIENT_ID;

    ///
    /// \brief Maximum numbers of blacklist loggers possible
    ///
    static const int MAX_BLACKLIST_LOGGERS;

    Configuration();
    explicit Configuration(const std::string& configurationFile);

    bool save(const std::string& outputFile);
    std::string exportAsString();
    void load(const std::string& configurationFile);
    void loadFromInput(std::string&& jsonStr);
    bool validateConfigFile(const std::string& filename) const;

    inline void reload()
    {
        RLOG(INFO) << "Reloading configurations...";
        load(m_configurationFile);
    }

    void addLoggerFlag(const std::string& loggerId, Flag flag);

    inline bool hasFlag(Flag flag) const
    {
        return m_flag != 0 && (m_flag & flag) != 0;
    }

    inline unsigned int flag() const
    {
        return m_flag;
    }

    inline void addFlag(Flag flag)
    {
        m_flag |= flag;
    }

    inline void removeFlag(Flag flag)
    {
        m_flag &= ~flag;
    }

    inline std::string configurationFile() const
    {
        return m_configurationFile;
    }

    inline int adminPort() const
    {
        return m_adminPort;
    }

    inline int connectPort() const
    {
        return m_connectPort;
    }

    inline int loggingPort() const
    {
        return m_loggingPort;
    }

    inline unsigned int defaultKeySize() const
    {
        return m_defaultKeySize;
    }

    inline unsigned int clientAge() const
    {
        return m_clientAge;
    }

    inline unsigned int timestampValidity() const
    {
        return m_timestampValidity;
    }

    inline unsigned int dispatchDelay() const
    {
        return m_dispatchDelay;
    }

    inline unsigned int maxItemsInBulk() const
    {
        return m_maxItemsInBulk;
    }

    inline unsigned int nonAcknowledgedClientAge() const
    {
        return m_nonAcknowledgedClientAge;
    }

    inline unsigned int clientIntegrityTaskInterval() const
    {
        return m_clientIntegrityTaskInterval;
    }

    inline const std::unordered_map<std::string, RotationFrequency>& rotationFreqencies() const
    {
        return m_rotationFrequencies;
    }

    inline bool isMalformedJson() const
    {
        return m_isMalformedJson;
    }
    inline bool isValid() const
    {
        return m_isValid;
    }
    inline std::string errors() const
    {
        return m_errors;
    }

    inline bool isManagedLogger(const std::string& loggerId) const
    {
        return m_configurations.find(loggerId) != m_configurations.end();
    }

    inline bool isManagedLoggerForClient(const std::string& clientId, const std::string& loggerId) const
    {
        return m_managedClientsLoggers.find(clientId) != m_managedClientsLoggers.end() &&
                m_managedClientsLoggers.at(clientId).find(loggerId) != m_managedClientsLoggers.at(clientId).end();
    }

    inline const std::unordered_map<std::string, unsigned int>& keySizes() const
    {
        return m_keySizes;
    }

    inline const std::unordered_map<std::string, std::unordered_set<std::string>>& managedClientsLoggers() const
    {
        return m_managedClientsLoggers;
    }

    inline const std::unordered_map<std::string, std::string>& managedClientDefaultLogger() const
    {
        return m_managedClientDefaultLogger;
    }

    inline std::vector<Extension*>& logExtensions()
    {
        return m_logExtensions;
    }

    inline std::vector<Extension*>& preArchiveExtensions()
    {
        return m_preArchiveExtensions;
    }

    inline std::vector<Extension*>& postArchiveExtensions()
    {
        return m_postArchiveExtensions;
    }

    inline std::vector<Extension*>& dispatchErrorExtensions()
    {
        return m_dispatchErrorExtensions;
    }

    inline const std::unordered_map<std::string, std::pair<std::string, std::string>>& managedClientsKeys() const
    {
        return m_managedClientsKeys;
    }

    inline bool isBlacklisted(const std::string& loggerId) const
    {
        return std::find(m_blacklist.begin(), m_blacklist.end(), loggerId) != m_blacklist.end();
    }

    inline std::string serverKey() const
    {
        return m_serverKey;
    }

    inline const RSA::KeyPair& serverRSAKey() const
    {
        return m_serverRSAKey;
    }

    inline const std::string& serverRSASecret() const
    {
        return m_serverRSASecret;
    }

    inline unsigned int keySize(const std::string& clientId) const
    {
        if (clientId.empty() || m_keySizes.find(clientId) == m_keySizes.end()) {
            return m_defaultKeySize;
        }
        return m_keySizes.at(clientId);
    }

    std::string getConfigurationFile(const std::string&) const;

    void updateUnmanagedLoggerUserFromRequest(const std::string& loggerId, const LogRequest* request = nullptr);

    std::string getArchivedLogDirectory(const std::string&) const;
    std::string getArchivedLogFilename(const std::string&) const;
    std::string getArchivedLogCompressedFilename(const std::string&) const;
    RotationFrequency getRotationFrequency(const std::string&) const;

    bool hasLoggerFlag(const std::string& loggerId, Flag flag) const;

    inline std::string managedLoggersEndpoint() const
    {
        return m_managedLoggersEndpoint;
    }

    inline std::string managedClientsEndpoint() const
    {
        return m_managedClientsEndpoint;
    }

    std::string findLoggerUser(const std::string& loggerId) const;

    inline unsigned int fileMode() const
    {
        return m_fileMode;
    }

    inline void setHomePath(const std::string& homePath)
    {
        m_homePath = homePath;
    }

private:
    JsonDoc m_jsonDoc;
    std::string m_configurationFile;

    int m_adminPort;
    int m_connectPort;
    int m_loggingPort;

    unsigned int m_flag;

    std::unordered_map<std::string, std::string> m_configurations;
    std::unordered_map<std::string, std::string> m_archivedLogsDirectories;
    std::unordered_map<std::string, std::string> m_archivedLogsFilenames;
    std::unordered_map<std::string, std::string> m_archivedLogsCompressedFilenames;
    std::unordered_map<std::string, RotationFrequency> m_rotationFrequencies;
    std::unordered_map<std::string, Flag> m_loggerFlags;
    std::unordered_map<std::string, unsigned int> m_keySizes;
    std::unordered_set<std::string> m_blacklist;
    std::unordered_set<std::string> m_remoteManagedClients;
    std::unordered_set<std::string> m_remoteManagedLoggers;
    std::vector<Extension*> m_logExtensions;
    std::vector<Extension*> m_preArchiveExtensions;
    std::vector<Extension*> m_postArchiveExtensions;
    std::vector<Extension*> m_dispatchErrorExtensions;

    std::unordered_map<std::string, std::pair<std::string, std::string>> m_managedClientsKeys;
    std::unordered_map<std::string, std::unordered_set<std::string>> m_managedClientsLoggers;
    std::unordered_map<std::string, std::string> m_managedLoggerUserMap;
    std::unordered_map<std::string, std::string> m_unmanagedLoggerUserMap;
    std::unordered_map<std::string, std::string> m_managedClientDefaultLogger;

    unsigned int m_nonAcknowledgedClientAge;
    unsigned int m_clientAge;
    unsigned int m_timestampValidity;
    unsigned int m_dispatchDelay;
    unsigned int m_clientIntegrityTaskInterval;
    unsigned int m_maxItemsInBulk;
    unsigned int m_defaultKeySize;
    unsigned int m_fileMode;

    std::string m_archivedLogDirectory;
    std::string m_archivedLogFilename;
    std::string m_archivedLogCompressedFilename;

    std::string m_serverKey;
    RSA::KeyPair m_serverRSAKey;
    // only for saving reference
    std::string m_serverRSAPublicKeyFile;
    std::string m_serverRSAPrivateKeyFile;
    std::string m_serverRSASecret;
    std::string m_managedLoggersEndpoint;
    std::string m_managedClientsEndpoint;

    std::string m_errors;
    bool m_isValid;
    bool m_isMalformedJson;

    std::mutex m_mutex;

    std::string m_homePath;

    friend class Clients;
    friend class Setup;

    bool addManagedClient(const std::string& clientId, const std::string& publicKey);
    bool verifyManagedClient(const std::string& clientId, const std::string& signature) const;
    void removeManagedClient(const std::string& clientId);

    inline bool isManagedClient(const std::string& clientId) const
    {
        return m_managedClientsKeys.find(clientId) != m_managedClientsKeys.end();
    }

    void loadManagedLoggers(const JsonDoc& json, std::stringstream& errorStream, bool viaUrl);
    void loadManagedClients(const JsonDoc& json, std::stringstream& errorStream, bool viaUrl);
    void loadLoggersBlacklist(const JsonDoc& json, std::stringstream& errorStream);

    void loadExtensions(const JsonDoc& json, std::stringstream& errorStream);
};
}
#endif /* Configuration_h */
