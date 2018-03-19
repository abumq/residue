//
//  configuration.cc
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

#include "core/configuration.h"

#include <sys/stat.h>
#include <pwd.h>

#include <set>
#include <fstream>
#include <utility>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "core/json-builder.h"
#include "core/json-doc.h"
#include "core/residue-exception.h"
#include "crypto/aes.h"
#include "crypto/base16.h"
#include "crypto/rsa.h"
#include "extensions/extension.h"
#include "logging/log-request.h"
#include "logging/log.h"
#include "net/http-client.h"
#include "utils/utils.h"

using namespace residue;

const std::string Configuration::UNMANAGED_CLIENT_ID = "unmanaged";
const int Configuration::MAX_BLACKLIST_LOGGERS = 10000;

struct ExtensionMap
{
    Extension::Type type;
    std::string name;
    std::vector<Extension*>* data;
};

Configuration::Configuration(const std::string& configurationFile) :
    m_configurationFile(configurationFile),
    m_flag(0x0),
    m_isValid(true),
    m_isMalformedJson(false)
{
    load(m_configurationFile);
}

Configuration::Configuration() :
    m_flag(0x0),
    m_isValid(true),
    m_isMalformedJson(false)
{

}

void Configuration::loadFromInput(std::string&& jsonStr)
{
    // Clean start
    m_errors = "";
    m_serverKey = "";
    m_flag = 0x0;

    m_configurations.clear();
    m_archivedLogsDirectories.clear();
    m_archivedLogsFilenames.clear();
    m_archivedLogCompressedFilename.clear();
    m_rotationFrequencies.clear();
    m_loggerFlags.clear();
    m_blacklist.clear();
    m_managedClientsEndpoint.clear();
    m_managedClientsKeys.clear();
    m_managedClientsLoggers.clear();
    m_managedLoggersEndpoint.clear();
    m_remoteManagedClients.clear();
    m_remoteManagedLoggers.clear();
    m_serverRSASecret.clear();
    m_serverRSAPrivateKeyFile.clear();
    m_serverRSAPublicKeyFile.clear();
 #ifdef RESIDUE_USE_MINE
    m_serverRSAKey.privateKey = RSA::PrivateKey();
    m_serverRSAKey.publicKey = RSA::PublicKey();
 #else
    m_serverRSAKey.privateKey.clear();
    m_serverRSAKey.publicKey.clear();
 #endif
    m_managedLoggerUserMap.clear();
    m_managedClientDefaultLogger.clear();
    m_logExtensions.clear();
    m_preArchiveExtensions.clear();
    m_postArchiveExtensions.clear();
    m_dispatchErrorExtensions.clear();
    m_isMalformedJson = false;
    m_isValid = true;

    std::stringstream errorStream;

    m_jsonDoc.parse(jsonStr);

    if (!m_jsonDoc.isValid()) {
        m_isMalformedJson = true;
        m_isValid = false;
        m_errors = m_jsonDoc.errorText();
        return;
    }
    m_adminPort = m_jsonDoc.get<int>("admin_port", 8776);
    m_connectPort = m_jsonDoc.get<int>("connect_port", 8777);
    m_loggingPort = m_jsonDoc.get<int>("logging_port", 8778);
    m_isValid = m_adminPort > 0 && m_connectPort > 0 && m_loggingPort > 0;


    if (!m_isValid) {
        errorStream  << "  Invalid port(s). Please choose all 3 valid ports." << std::endl;
    }

    if (m_jsonDoc.get<bool>("enable_cli", true)) {
        addFlag(Configuration::Flag::ENABLE_CLI);
    }
    if (m_jsonDoc.get<bool>("allow_unmanaged_loggers", true)) {
        addFlag(Configuration::Flag::ALLOW_UNMANAGED_LOGGERS);
    }
    if (m_jsonDoc.get<bool>("allow_insecure_connection", true)) {
        addFlag(Configuration::ALLOW_INSECURE_CONNECTION);
    }
    if (m_jsonDoc.get<bool>("compression", true)) {
        addFlag(Configuration::COMPRESSION);
    }
    if (m_jsonDoc.get<bool>("allow_unmanaged_clients", true)) {
        addFlag(Configuration::Flag::ALLOW_UNMANAGED_CLIENTS);
    }
    if (m_jsonDoc.get<bool>("allow_bulk_log_request", true)) {
        addFlag(Configuration::Flag::ALLOW_BULK_LOG_REQUEST);
    }
    if (m_jsonDoc.get<bool>("immediate_flush", true)) {
        addFlag(Configuration::Flag::IMMEDIATE_FLUSH);
    }
    if (m_jsonDoc.get<bool>("requires_timestamp", true)) {
        addFlag(Configuration::Flag::REQUIRES_TIMESTAMP);
    } else {
        RLOG(WARNING) << "You have disabled 'requires_timestamp'. Your server is prone to replay attack.";
    }

    if (m_jsonDoc.get<bool>("enable_dynamic_buffer", false)) {
        addFlag(Configuration::Flag::ENABLE_DYNAMIC_BUFFER);
        RVLOG(RV_INFO) << "Dyanmic buffer enabled";
    }

    m_serverKey = m_jsonDoc.get<std::string>("server_key", AES::generateKey(256));

    if (m_serverKey.size() != 64) {
        errorStream << "  Invalid value for [server_key]. It should be hex-value of 256-bit key. "
                    << "e.g, 048CB7050312DB329788CE1533C294A1F248F8A1BD6F611D7516803EDE271C65. "
                    << "Use mine to generate a key: [mine -g --aes 256]" << std::endl;
    }

    m_serverRSAPrivateKeyFile = m_jsonDoc.get<std::string>("server_rsa_private_key", "");
    std::string rsaPrivateKeySecret = Base16::decode(m_jsonDoc.get<std::string>("server_rsa_secret", ""));
    if (!m_serverRSAPrivateKeyFile.empty()) {

        m_serverRSAPublicKeyFile = m_jsonDoc.get<std::string>("server_rsa_public_key", "");
        if (m_serverRSAPublicKeyFile.empty()) {
            errorStream << "  Both RSA private and public keys must be provided if provided at all" << std::endl;
        } else {
            Utils::resolveResidueHomeEnvVar(m_serverRSAPrivateKeyFile, m_homePath);
            Utils::resolveResidueHomeEnvVar(m_serverRSAPublicKeyFile, m_homePath);

            if (!Utils::fileExists(m_serverRSAPrivateKeyFile.c_str()) || !Utils::fileExists(m_serverRSAPublicKeyFile.c_str())) {
                errorStream << "  RSA private key or public key does not exist" << std::endl;
            } else {
                std::ifstream rsaPrivateKeyStream(m_serverRSAPrivateKeyFile);
                std::string rsaPrivateKey((std::istreambuf_iterator<char>(rsaPrivateKeyStream)),
                                    (std::istreambuf_iterator<char>()));
                rsaPrivateKeyStream.close();
                std::ifstream rsaPublicKeyStream(m_serverRSAPublicKeyFile);
                std::string rsaPublicKey((std::istreambuf_iterator<char>(rsaPublicKeyStream)),
                                    (std::istreambuf_iterator<char>()));
                rsaPublicKeyStream.close();

                auto prikey = RSA::loadPrivateKey(rsaPrivateKey, rsaPrivateKeySecret);
                auto pubkey = RSA::loadPublicKey(rsaPublicKey);

                if (!RSA::verifyKeyPair(prikey, pubkey, rsaPrivateKeySecret)) {
                    errorStream << "  Verify server key: Invalid RSA key pair.";
                } else {
                    m_serverRSAKey.privateKey = prikey;
                    m_serverRSAKey.publicKey = pubkey;
                    m_serverRSASecret = rsaPrivateKeySecret;
                }
            }
        }
    } else if (!hasFlag(Configuration::ALLOW_INSECURE_CONNECTION)) {
        errorStream << "  Server does not allow plain connections. Please provide RSA key pair" << std::endl;
    }

    m_archivedLogDirectory = m_jsonDoc.get<std::string>("archived_log_directory", "");
    if (m_archivedLogDirectory.empty()) {
        errorStream << "  Please choose valid default archived_log_directory" << std::endl;
    }

    m_archivedLogFilename = m_jsonDoc.get<std::string>("archived_log_filename", "");
    if (m_archivedLogFilename.empty()) {
        errorStream << "  Please choose valid default archived_log_filename" << std::endl;
    }

    m_archivedLogCompressedFilename = m_jsonDoc.get<std::string>("archived_log_compressed_filename", "");
    if (m_archivedLogCompressedFilename.empty() || m_archivedLogCompressedFilename == m_archivedLogFilename
            || m_archivedLogCompressedFilename.find("/") != std::string::npos
            || m_archivedLogCompressedFilename.find("\\") != std::string::npos) {
        errorStream << "  Please choose valid default archived_log_compressed_filename" << std::endl;
    }

    m_clientAge = m_jsonDoc.get<unsigned int>("client_age", 259200);
    if (m_clientAge != 0 && m_clientAge < 120) {
        RLOG(WARNING) << "Invalid value for [client_age]. Setting it to minimum [120]";
        m_clientAge = 120;
    }
    m_defaultKeySize = m_jsonDoc.get<unsigned int>("default_key_size", 256);
    if (m_defaultKeySize != 128 && m_defaultKeySize != 192 && m_defaultKeySize != 256) {
        errorStream << "  Invalid default key size. Please choose 128, 192 or 256-bit" << std::endl;
    }

    auto hasFileMode = [&](unsigned int mode) {
        return m_fileMode != 0 && (m_fileMode & mode) != 0;
    };

    m_fileMode = m_jsonDoc.get<unsigned int>("file_mode", static_cast<unsigned int>(S_IRUSR | S_IWUSR | S_IRGRP));
    if (hasFileMode(static_cast<unsigned int>(S_IWOTH)) || hasFileMode(static_cast<unsigned int>(S_IXOTH))) {
        errorStream << "  File mode too open [" << m_fileMode << "]. You should at least not allow others to write to the file." << std::endl;
    } else if (!hasFileMode(static_cast<unsigned int>(S_IRUSR))) {
        errorStream << "  File mode invalid [" << m_fileMode << "]. User must be able to read files" << std::endl;
    } else if (!hasFileMode(static_cast<unsigned int>(S_IWUSR))) {
        errorStream << "  File mode invalid [" << m_fileMode << "]. User must be able to write files" << std::endl;
    }
    m_nonAcknowledgedClientAge = m_jsonDoc.get<unsigned int>("non_acknowledged_client_age", 300);
    if (m_nonAcknowledgedClientAge < 120) {
        RLOG(WARNING) << "Invalid value for [non_acknowledged_client_age]. Setting it to default [120]";
        m_nonAcknowledgedClientAge = 120;
    }
    m_timestampValidity = m_jsonDoc.get<unsigned int>("timestamp_validity", 120);
    if (m_timestampValidity < 30 || m_timestampValidity > 86400) {
        RLOG(WARNING) << "Invalid value for [timestamp_validity]. Setting it to minimum [30]";
        m_timestampValidity = 30;
    }

    unsigned int defaultClientIntegrityTaskInterval = std::max(300U, std::min(m_clientAge, m_nonAcknowledgedClientAge));
    m_clientIntegrityTaskInterval = m_jsonDoc.get<unsigned int>("client_integrity_task_interval", defaultClientIntegrityTaskInterval);
    if (m_clientIntegrityTaskInterval == 0 || m_clientIntegrityTaskInterval < std::min(m_clientAge, m_nonAcknowledgedClientAge)) {
        RLOG(WARNING) << "Invalid value for [client_integrity_task_interval (" << m_clientIntegrityTaskInterval << ")]. "
                      << "Choose anything greater than or equal to " << std::min(m_clientAge, m_nonAcknowledgedClientAge) << " but not zero. Setting it to lower ["
                      << std::min(m_clientAge, m_nonAcknowledgedClientAge) << "]";
        m_clientIntegrityTaskInterval = defaultClientIntegrityTaskInterval;
    }
    m_dispatchDelay = m_jsonDoc.get<unsigned int>("dispatch_delay", 1);
    if (m_dispatchDelay > 500) {
        RLOG(WARNING) << "Invalid value for [dispatch_delay]. Setting it to default [1ms]";
        m_dispatchDelay = 1;
    }
    m_maxItemsInBulk = m_jsonDoc.get<unsigned int>("max_items_in_bulk", 5);
    if (m_maxItemsInBulk <= 1 || m_maxItemsInBulk >= 100) {
        errorStream << "  Invalid value for [max_items_in_bulk]. Please choose between 2-100" << std::endl;
    }


    // We load managed loggers before managed clients because
    // managed clients may have "loggers" array
    // that will be cross-checked with loggers list

    if (m_jsonDoc.hasKey("managed_loggers")) {
        loadManagedLoggers(m_jsonDoc.getArr("managed_loggers"), errorStream, false);
    }

    auto queryEndpoint = [&](const std::string& endpoint,
            const std::string& keyName,
            const std::function<void(const JsonDoc&)>& cb) {

        RVLOG(RV_INFO) << "Querying [" << endpoint << "]...";
        std::string contents;

        try {
            contents = HttpClient::fetchUrlContents(endpoint);
            Utils::trim(contents);
            if (!contents.empty()) {
                JsonDoc j(contents);
                if (j.isValid()) {
                    if (j.hasKey(keyName.c_str())) {
                        cb(j.getObj(keyName.c_str()));
                    } else {
                        errorStream << endpoint << " does not contain " << keyName << std::endl;
                    }
                } else {
                    errorStream << "  Invalid JSON at " << endpoint << ", JSON: " << contents << std::endl;
                }
            }
        } catch (ResidueException& e) {
            errorStream << "  URL error: " << e.what() << std::endl;
        }
    };

    if (m_jsonDoc.hasKey("managed_loggers_endpoint")) {
        m_managedLoggersEndpoint = m_jsonDoc.get<std::string>("managed_loggers_endpoint", "");
        if (!m_managedLoggersEndpoint.empty()) {
            queryEndpoint(m_managedLoggersEndpoint, "managed_loggers", [&](const JsonDoc& json) {
                loadManagedLoggers(json, errorStream, true);
            });
        }
    }

    if (m_jsonDoc.hasKey("managed_clients")) {
        loadManagedClients(m_jsonDoc.getArr("managed_clients"), errorStream, false);
    }

    if (m_jsonDoc.hasKey("managed_clients_endpoint")) {
        m_managedClientsEndpoint = m_jsonDoc.get<std::string>("managed_clients_endpoint", "");
        if (!m_managedClientsEndpoint.empty()) {
            queryEndpoint(m_managedClientsEndpoint, "managed_clients", [&](const JsonDoc& json) {
                loadManagedClients(json, errorStream, true);
            });
        }
    }

    JsonDoc jLoggersBlacklist(m_jsonDoc.getArr("loggers_blacklist"));
    if (jLoggersBlacklist.isArray()) {
        loadLoggersBlacklist(jLoggersBlacklist, errorStream);
    }


 #ifdef RESIDUE_HAS_EXTENSIONS
    if (m_jsonDoc.hasKey("extensions")) {
        loadExtensions(m_jsonDoc.getArr("extensions"), errorStream);
    }
 #endif

 #ifndef RESIDUE_HAS_CURL
    if (!m_managedClientsEndpoint.empty() || !m_managedLoggersEndpoint.empty()) {
        RLOG(WARNING) << "This residue build does not support HTTPS endpoint urls";
    }
 #endif
    if (getConfigurationFile("default").empty()) {
        errorStream << "  Configuration file for required logger [default] is not specified" << std::endl;
    }
    if (getConfigurationFile(RESIDUE_LOGGER_ID).empty()) {
        errorStream << "  Configuration file for required logger [" << RESIDUE_LOGGER_ID << "] is not specified" << std::endl;
    }
    m_errors = errorStream.str();
    m_isValid = m_errors.empty();
}


void Configuration::loadManagedLoggers(const JsonDoc& json, std::stringstream& errorStream, bool viaUrl)
{
    for (const auto& logger : json) {
        JsonDoc j(logger);

        std::string loggerId = j.get<std::string>("logger_id", "");
        if (loggerId.empty()) {
            errorStream << "  Logger ID not provided in managed_loggers" << std::endl;
            continue;
        }
        if (m_configurations.find(loggerId) != m_configurations.end()) {
            errorStream << "  Duplicate logger in managed_loggers [" << loggerId << "]" << std::endl;
            continue;
        }
        std::string easyloggingConfigFile = j.get<std::string>("configuration_file", "");
        if (!easyloggingConfigFile.empty()) {
            Utils::resolveResidueHomeEnvVar(easyloggingConfigFile, m_homePath);
            if (!Utils::fileExists(easyloggingConfigFile.c_str())) {
                errorStream << "  File [" << easyloggingConfigFile << "] does not exist" << std::endl;
                continue;
            }
            // validate configuration file
            if (!validateConfigFile(easyloggingConfigFile)) {
                errorStream << "  Easylogging++ configuration file [ "
                            << easyloggingConfigFile << "] contains configurations." << std::endl;
                continue;
            }
            m_configurations.insert(std::make_pair(loggerId, easyloggingConfigFile));
            if (viaUrl) {
                m_remoteManagedLoggers.insert(loggerId);
            }

            // load users before configuring

            std::string loggerUser = j.get<std::string>("user", "");
            if (!loggerUser.empty()) {
                struct passwd* userpwd = getpwnam(loggerUser.data());
                if (userpwd == nullptr) {
                    errorStream << "  User corresponding to logger [" << loggerId << "] does not exist [" << loggerUser << "]" << std::endl;
                    endpwent();
                    continue;
                }
                endpwent();
                m_managedLoggerUserMap.insert(std::make_pair(loggerId, loggerUser));
            }

            // load logger and configure
            el::Configurations confs(easyloggingConfigFile);
            el::Logger* logger = el::Loggers::getLogger(loggerId);
            el::base::type::EnumType lIndex = el::LevelHelper::kMinValid;
            el::Loggers::reconfigureLogger(logger, confs);
            std::vector<std::string> doneList;
            el::LevelHelper::forEachLevel(&lIndex, [&](void) -> bool {
                el::Configuration* filenameConf = confs.get(el::LevelHelper::castFromInt(lIndex), el::ConfigurationType::Filename);
                if (filenameConf != nullptr && std::find(doneList.begin(), doneList.end(), filenameConf->value()) == doneList.end()) {
                    doneList.push_back(filenameConf->value());
                    Utils::updateFilePermissions(filenameConf->value().data(), logger, this);
                }
                return false; // don't exit yet
            });
        } else {
            errorStream << "  Please specify Easylogging++ configuration for known logger [" << loggerId << "]" << std::endl;
            continue;
        }

        std::string rotationFreq = j.get<std::string>("rotation_freq", "");
        Utils::toUpper(rotationFreq);
        if (!rotationFreq.empty() && rotationFreq != "NEVER") {
            RotationFrequency frequency = RotationFrequency::NEVER;
            if (rotationFreq == "HOURLY") {
                frequency = RotationFrequency::HOURLY;
            } else if (rotationFreq == "DAILY") {
                frequency = RotationFrequency::DAILY;
            } else if (rotationFreq == "SIX_HOURS") {
                frequency = RotationFrequency::SIX_HOURS;
            } else if (rotationFreq == "TWELVE_HOURS") {
                frequency = RotationFrequency::TWELVE_HOURS;
            } else if (rotationFreq == "WEEKLY") {
                frequency = RotationFrequency::WEEKLY;
            } else if (rotationFreq == "MONTHLY") {
                frequency = RotationFrequency::MONTHLY;
            } else if (rotationFreq == "YEARLY") {
                frequency = RotationFrequency::YEARLY;
            } else {
                errorStream << "  Invalid rotation frequency [" << rotationFreq << "]" << std::endl;
            }
            m_rotationFrequencies.insert(std::make_pair(loggerId, frequency));
        }

        std::string archivedLogFilename = j.get<std::string>("archived_log_filename", "");
        if (!archivedLogFilename.empty()) {
            m_archivedLogsFilenames.insert(std::make_pair(loggerId, "%logger-" + archivedLogFilename));
        }

        std::string archivedLogCompressedFilename = j.get<std::string>("archived_log_compressed_filename", "");
        if (!archivedLogCompressedFilename.empty()) {
            if (archivedLogCompressedFilename.find("/") != std::string::npos || archivedLogCompressedFilename.find("\\") != std::string::npos) {
                errorStream << "  archived_log_compressed_filename contains illegal character (path character). It should be pure filename format." << std::endl;
            } else {
                m_archivedLogsCompressedFilenames.insert(std::make_pair(loggerId, archivedLogCompressedFilename));
            }
        }

        std::string archivedLogDirectory = j.get<std::string>("archived_log_directory", "");
        if (!archivedLogDirectory.empty()) {
            m_archivedLogsDirectories.insert(std::make_pair(loggerId, archivedLogDirectory));
        }
    }
}

void Configuration::loadManagedClients(const JsonDoc& json, std::stringstream& errorStream, bool viaUrl)
{
    for (const auto& managedClientPair : json) {
        JsonDoc j(managedClientPair);
        std::string clientId = j.get<std::string>("client_id", "");
        if (clientId.empty()) {
            errorStream << "  Client ID not provided in managed_clients" << std::endl;
            continue;
        }
        if (!Utils::isAlphaNumeric(clientId, "-_@#.")) {
            errorStream << "  Invalid character in client ID, should be alpha-numeric (can also include these characters excluding square brackets: [_@-#.])" << std::endl;
            continue;
        }
        if (clientId == UNMANAGED_CLIENT_ID) {
            errorStream << "  " << UNMANAGED_CLIENT_ID << " is invalid name for client" << std::endl;
            continue;
        }
        std::string publicKey = j.get<std::string>("public_key", "");
        if (publicKey.empty()) {
            errorStream << "  RSA public key not provided in managed_clients for [" << clientId << "]" << std::endl;
            continue;
        }
        Utils::resolveResidueHomeEnvVar(publicKey, m_homePath);
        if (m_managedClientsKeys.find(clientId) != m_managedClientsKeys.end()) {
            errorStream << "  Duplicate client ID in managed_clients [" << clientId << "]" << std::endl;
            continue;
        }
        if (!Utils::fileExists(publicKey.c_str())) {
            errorStream << "  Public key [" << publicKey << "] for client [" << clientId << "] does not exist" << std::endl;
            continue;
        }

        std::ifstream fs(publicKey, std::ios::in);
        if (!fs.is_open()) {
            errorStream << "Public key file not readable";
            continue;
        }
        std::string publicKeyContents = std::string(std::istreambuf_iterator<char>(fs),
                                                    std::istreambuf_iterator<char>());

        m_managedClientsKeys.insert(std::make_pair(clientId, std::make_pair(publicKey, publicKeyContents)));

        if (viaUrl) {
            m_remoteManagedClients.insert(clientId);
        }

        unsigned int keySize = j.get<unsigned int>("key_size", 0);

        if (keySize == 128 || keySize == 192 || keySize == 256) {
            m_keySizes.insert(std::make_pair(clientId, keySize));
        } else {
            if (keySize != 0) {
                errorStream << "  Invalid key size [" << keySize << "] for client [" << clientId << "]. Please choose 128, 192 or 256-bit" << std::endl;
            }
        }

        JsonDoc loggers(j.getArr("loggers"));

        if (loggers.isArray()) {
            std::unordered_set<std::string> loggerIds;
            for (const auto& loggerNode : loggers) {
                JsonDoc jLoggers(loggerNode);
                std::string loggerId = jLoggers.as<std::string>("");
                if (loggerId.empty()) {
                    continue;
                }
                if (!isManagedLogger(loggerId)) {
                    errorStream << "  Logger [" << loggerId << "] for client [" << clientId << "] is unmanaged" << std::endl;
                    continue;
                }
                loggerIds.insert(loggerId);
            }
            m_managedClientsLoggers.insert(std::make_pair(clientId, loggerIds));

            std::string defaultLogger = j.get<std::string>("default_logger", "");
            if (!defaultLogger.empty()) {
                if (loggerIds.find(defaultLogger) != loggerIds.end()) {
                    m_managedClientDefaultLogger.insert(std::make_pair(clientId, defaultLogger));
                } else {
                    errorStream << "  Default logger ["  << defaultLogger << "] for client [" << clientId << "] is not part of [loggers] array";
                }
            }

            std::string loggerUser = j.get<std::string>("user", "");
            if (!loggerUser.empty()) {
                struct passwd* userpwd = getpwnam(loggerUser.data());
                if (userpwd == nullptr) {
                    errorStream << "  User corresponding to client [" << clientId << "] does not exist [" << loggerUser << "]" << std::endl;
                    endpwent();
                    continue;
                }
                endpwent();
                for (std::string loggerId : loggerIds) {
                    if (m_managedLoggerUserMap.find(loggerId) == m_managedLoggerUserMap.end()) {
                        m_managedLoggerUserMap.insert(std::make_pair(loggerId, loggerUser));
                    } else {
                        // for same user ignore, for different user this is a config warning
                        std::string existingAssignedUser = m_managedLoggerUserMap.at(loggerId);
                        if (existingAssignedUser != loggerUser) {
                            RLOG(WARNING) << "  User for logger [" << loggerId << "] has explicit user [" << existingAssignedUser << "]";
                        }
                    }
                }
            }

        } else {
            // no loggers array
            std::string defaultLogger = j.get<std::string>("default_logger", "");
            if (!defaultLogger.empty()) {
                errorStream << "  Default logger ["  << defaultLogger << "] for client [" << clientId << "] is not part of [loggers] array. Please see https://github.com/muflihun/residue/blob/master/docs/CONFIGURATION.md#managed_clientsloggers";
            }
        }
    }
}

void Configuration::loadLoggersBlacklist(const JsonDoc& json, std::stringstream& errorStream)
{
    for (const auto& loggerId : json) {
        JsonDoc j(loggerId);
        std::string loggerIdStr = j.as<std::string>("");
        if (loggerIdStr.empty()) {
            continue;
        }
        if (isManagedLogger(loggerIdStr)) {
            errorStream << "  Cannot blacklist [" << loggerId << "] logger. Remove it from 'managed_loggers' first." << std::endl;
            continue;
        }
        if (m_blacklist.size() >= MAX_BLACKLIST_LOGGERS) {
            errorStream << "  You have added maximum number of blacklisted loggers. Please consider using 'allow_unmanaged_loggers' instead." << std::endl;
            continue;
        }
        if (std::find(m_blacklist.begin(), m_blacklist.end(), loggerIdStr) == m_blacklist.end()) {
            m_blacklist.insert(loggerIdStr);
        }
    }
}

bool Configuration::save(const std::string& outputFile)
{
    std::fstream file;
    file.open(outputFile, std::ofstream::out);
    if (!file) {
        RLOG(ERROR) << "Unable to open file for writing [" << outputFile << "]";
        return false;
    }
    file << m_jsonDoc.dump(4);
    file.flush();
    file.close();
    return true;
}

bool Configuration::validateConfigFile(const std::string& filename) const
{
    std::ifstream fs(filename, std::ifstream::in);
    if (!fs.is_open()) {
        return false;
    }
    bool result = true;
    std::string line;
    static const std::vector<std::string> ILLEGAL_CONFIGS {
        el::base::consts::kConfigurationLoggerId,
        el::ConfigurationTypeHelper::convertToString(el::ConfigurationType::MaxLogFileSize),
        el::ConfigurationTypeHelper::convertToString(el::ConfigurationType::LogFlushThreshold)
    };
    while (fs.good()) {
      std::getline(fs, line);
      std::for_each(ILLEGAL_CONFIGS.begin(), ILLEGAL_CONFIGS.end(), [&](const std::string& illegalConfig) {
          // We ignore "--" in easylogging++ configuration
          if (Utils::startsWith(Utils::trim(line), illegalConfig)) {
              result = false;
              return;
          }
      });
    }
    fs.close();
    return result;
}

void Configuration::load(const std::string& configurationFile)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_configurationFile = configurationFile;
    std::stringstream errorStream;
    std::ifstream jsonFile(configurationFile);
    if (!jsonFile.is_open()) {
        errorStream << "  Configuration file [" << configurationFile << "] does not exist.";
        m_isValid = false;
        m_errors = errorStream.str();
        return;
    }
    std::string jsonStr((std::istreambuf_iterator<char>(jsonFile)),
                        (std::istreambuf_iterator<char>()));
    jsonFile.close();
    loadFromInput(std::move(jsonStr));
}

bool Configuration::hasLoggerFlag(const std::string& loggerId,
                                  Flag flag) const
{
    const auto& iter = m_loggerFlags.find(loggerId);
    if (iter != m_loggerFlags.end()) {
         return iter->second != 0 && (iter->second & flag) != 0;
    }
    return false;
}

bool Configuration::addManagedClient(const std::string& clientId,
                                   const std::string& publicKey)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_managedClientsKeys.find(clientId) != m_managedClientsKeys.end()) {
        RLOG(ERROR) << "Known client already exists";
        return false;
    }

    std::ifstream fs(publicKey, std::ios::in);
    if (!fs.is_open()) {
        RLOG(ERROR) << "Public key file not readable";
        return false;
    }
    std::string publicKeyContents = std::string(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());

    m_managedClientsKeys.insert(std::make_pair(clientId, std::make_pair(publicKey, publicKeyContents)));
    return true;
}

void Configuration::removeManagedClient(const std::string& clientId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto pos = m_managedClientsKeys.find(clientId);
    if (pos != m_managedClientsKeys.end()) {
        m_managedClientsKeys.erase(pos);
    }
}

bool Configuration::verifyManagedClient(const std::string& clientId, const std::string& signature) const
{
    if (!isManagedClient(clientId)) {
        return false;
    }
    RSA::PublicKey rsaPublicKey = RSA::loadPublicKey(m_managedClientsKeys.at(clientId).second); // second = public key contents
    try {
        return RSA::verify(clientId, signature, rsaPublicKey);
    } catch (std::exception& e) {
        RVLOG(RV_ERROR) << "Could not verify known client: " << e.what();
        return false;
    }
}

void Configuration::addLoggerFlag(const std::string& loggerId,
                                  Flag flag)
{
    const auto& iter = m_loggerFlags.find(loggerId);
    if (iter == m_loggerFlags.end()) {
        m_loggerFlags[loggerId] = flag;
    } else {
        iter->second = static_cast<Flag>(static_cast<unsigned int>(iter->second) | static_cast<unsigned int>(flag));
    }
}

std::string Configuration::getConfigurationFile(const std::string& loggerId) const
{
    DRVLOG(RV_DEBUG) << "Finding configuration file for [" << loggerId << "]";
    if (m_configurations.find(loggerId) != m_configurations.end()) {
        DRVLOG(RV_DEBUG) << "Configuration file found for [" << loggerId << "]";
        return m_configurations.at(loggerId);
    }
    return "";
}

void Configuration::updateUnmanagedLoggerUserFromRequest(const std::string& loggerId, const LogRequest* request)
{
    if (m_unmanagedLoggerUserMap.find(loggerId) != m_unmanagedLoggerUserMap.end()) {
        return; // already set
    }
    // get conf of client's default logger
    if (request != nullptr) {
        RVLOG(RV_INFO) << "Updating user for unmanaged logger [" << loggerId << "] using client [" << request->clientId() << "]";
        if (m_managedClientDefaultLogger.find(request->clientId()) != m_managedClientDefaultLogger.end()) {
            std::string defaultLoggerForClient = m_managedClientDefaultLogger.at(request->clientId());
            std::string user = findLoggerUser(defaultLoggerForClient);
            RVLOG(RV_INFO) << "Found user for unmanaged logger [" << loggerId << "] => [" << user << "]";
            m_unmanagedLoggerUserMap.insert(std::make_pair(loggerId, user));
        }
    }
}

std::string Configuration::getArchivedLogDirectory(const std::string& loggerId) const
{
    if (m_archivedLogsDirectories.find(loggerId) != m_archivedLogsDirectories.end()) {
        return m_archivedLogsDirectories.at(loggerId);
    }
    return m_archivedLogDirectory;
}

std::string Configuration::getArchivedLogFilename(const std::string& loggerId) const
{
    if (m_archivedLogsFilenames.find(loggerId) != m_archivedLogsFilenames.end()) {
        return m_archivedLogsFilenames.at(loggerId);
    }
    return m_archivedLogFilename;
}

std::string Configuration::getArchivedLogCompressedFilename(const std::string& loggerId) const
{
    if (m_archivedLogsCompressedFilenames.find(loggerId) != m_archivedLogsCompressedFilenames.end()) {
        return m_archivedLogsCompressedFilenames.at(loggerId);
    }
    return m_archivedLogCompressedFilename;
}

std::string Configuration::findLoggerUser(const std::string& loggerId) const
{
    if (m_managedLoggerUserMap.find(loggerId) != m_managedLoggerUserMap.end()) {
        return m_managedLoggerUserMap.at(loggerId);
    }
    if (m_unmanagedLoggerUserMap.find(loggerId) != m_unmanagedLoggerUserMap.end()) {
        return m_unmanagedLoggerUserMap.at(loggerId);
    }
    return "";
}

Configuration::RotationFrequency Configuration::getRotationFrequency(const std::string& loggerId) const
{
    if (m_rotationFrequencies.find(loggerId) != m_rotationFrequencies.end()) {
        return m_rotationFrequencies.at(loggerId);
    }
    return RotationFrequency::NEVER;
}

void Configuration::loadExtensions(const JsonDoc& json, std::stringstream& errorStream)
{
    const std::vector<ExtensionMap> REGISTERED_EXTENSIONS = {
        { Extension::Type::Log, "LOG", &m_logExtensions },
        { Extension::Type::PreArchive, "PRE_ARCHIVE", &m_preArchiveExtensions },
        { Extension::Type::PostArchive, "POST_ARCHIVE", &m_postArchiveExtensions },
        { Extension::Type::DispatchError, "DISPATCH_ERROR", &m_dispatchErrorExtensions }
    };

    std::vector<std::string> ext;

    for (const auto& extension : json) {
        JsonDoc j(extension);

        std::string name = j.get<std::string>("name", "");
        if (name.empty()) {
            errorStream << "  Must provide extension name" << std::endl;
            continue;
        }

        std::string module = j.get<std::string>("module", "");
        if (module.empty()) {
            errorStream << "  Module path not provided" << std::endl;
            continue;
        }

        RLOG(INFO) << "Loading [Extension<" << name << ">]";
        Extension* e = Extension::load(module.c_str());
        if (e == nullptr) {
            RLOG(ERROR) << "Extension [" << module << "] failed to load: " << std::strerror(errno);
            continue;
        }

        const ExtensionMap* mapItem = nullptr;
        for (auto& regExt : REGISTERED_EXTENSIONS) {
            if (regExt.type == e->m_type) {
                mapItem = &regExt;
            }
        }
        if (mapItem == nullptr) {
            errorStream << "  Unable to determine extension [" << name << "] type ["
                        << static_cast<unsigned int>(e->m_type) << "]";
            continue;
        }
        std::string uniqName = std::to_string(static_cast<int>(e->m_type)) + "/" + name;
        if (std::find(ext.begin(), ext.end(), uniqName) != ext.end()) {
            errorStream << "  Duplicate extension could not be loaded: " << name;
        } else {
            e->m_description = j.get<std::string>("description", "");
            e->m_modulePath = module;
            ext.push_back(uniqName);
            if (j.hasKey("config")) {
                JsonDoc jextConfig(j.getObj("config"));
                e->setConfig(std::move(jextConfig));
            }

            RLOG(INFO) << "Loaded [" << mapItem->name << "::Extension<" << name << ">] loaded @ " << e;

            mapItem->data->push_back(e);
        }
    }

    RLOG_IF(m_logExtensions.size() > 2, WARNING) << "You have " << m_logExtensions.size() << " log extensions enabled. "
                                                    "This may slow down the server's log processing depending upon the time it takes to execute the extension.";
}

std::string Configuration::exportAsString()
{
    const std::size_t capacity = 4096;
    char source[capacity];

    JsonBuilder j(source, capacity);
    DRVLOG(RV_DEBUG_2) << "Starting JSON serialization with [" << capacity << "] bytes";

    j.startObject();
    j.addValue("admin_port", adminPort());
    j.addValue("connect_port", connectPort());
    j.addValue("logging_port", loggingPort());
    j.addValue("server_key", serverKey());
    if (!m_serverRSAPrivateKeyFile.empty()) {
        j.addValue("server_rsa_private_key", m_serverRSAPrivateKeyFile);
    }
    if (!m_serverRSAPublicKeyFile.empty()) {
        j.addValue("server_rsa_public_key", m_serverRSAPublicKeyFile);
    }
    if (!serverRSASecret().empty()) {
        j.addValue("server_rsa_secret", serverRSASecret());
    }
    j.addValue("default_key_size", defaultKeySize());
    j.addValue("file_mode", fileMode());
    j.addValue("enable_cli", hasFlag(Configuration::Flag::ENABLE_CLI));
    j.addValue("allow_insecure_connection", hasFlag(Configuration::Flag::ALLOW_INSECURE_CONNECTION));
    j.addValue("allow_unmanaged_loggers", hasFlag(Configuration::Flag::ALLOW_UNMANAGED_LOGGERS));
    j.addValue("allow_unmanaged_clients", hasFlag(Configuration::Flag::ALLOW_UNMANAGED_CLIENTS));
    j.addValue("immediate_flush", hasFlag(Configuration::Flag::IMMEDIATE_FLUSH));
    j.addValue("requires_timestamp", hasFlag(Configuration::Flag::REQUIRES_TIMESTAMP));
    j.addValue("compression", hasFlag(Configuration::Flag::COMPRESSION));
    j.addValue("allow_bulk_log_request", hasFlag(Configuration::Flag::ALLOW_BULK_LOG_REQUEST));
    j.addValue("max_items_in_bulk", maxItemsInBulk());
    j.addValue("timestamp_validity", timestampValidity());
    j.addValue("client_age", clientAge());
    j.addValue("non_acknowledged_client_age", nonAcknowledgedClientAge());
    j.addValue("client_integrity_task_interval", clientIntegrityTaskInterval());
    j.addValue("dispatch_delay", dispatchDelay());
    j.addValue("archived_log_directory", m_archivedLogDirectory);
    j.addValue("archived_log_filename", m_archivedLogFilename);
    j.addValue("archived_log_compressed_filename", m_archivedLogCompressedFilename);
/*
    if (!m_logExtensions.empty()) {
        j.startObject("extensions");
        j.startArray("log_extensions");
        for (auto& e : m_logExtensions) {
            j.addValue(e->module());
        }

        j.endArray();
        j.endObject();
    }
*/

    j.startArray("loggers_blacklist");
    for (auto& e : m_blacklist) {
        j.addValue(e);
    }
    j.endArray(); // loggers_blacklist

    j.startArray("managed_clients");
    for (auto c : m_managedClientsKeys) {
        if (m_remoteManagedClients.find(c.first) != m_remoteManagedClients.end()) {
            // do not save known clients fetched by URL
            continue;
        }
        j.startObject();

        j.addValue("client_id", c.first);
        j.addValue("public_key", c.second.first); // .first = filename | .second = file contents
        if (m_keySizes.find(c.first) != m_keySizes.end()) {
            j.addValue("key_size", m_keySizes.at(c.first));
        }
        if (m_managedClientsLoggers.find(c.first) != m_managedClientsLoggers.end()) {
            const auto& list = m_managedClientsLoggers.at(c.first);
            if (!list.empty()) {
                j.startArray("loggers");
                for (const auto& loggerId : list) {
                    j.addValue(loggerId);
                }
                j.endArray();
            }
        }
/*
        if (m_managedClientUserMap.find(c.first) != m_managedClientUserMap.end()) {
            j.addValue("user", m_managedClientUserMap.at(c.first));
        }*/
        if (m_managedClientDefaultLogger.find(c.first) != m_managedClientDefaultLogger.end()) {
            j.addValue("default_logger", m_managedClientDefaultLogger.at(c.first));
        }
        j.endObject();
    }
    j.endArray(); // managed_clients

    if (!m_managedClientsEndpoint.empty()) {
        j.addValue("managed_clients_endpoint", m_managedClientsEndpoint);
    }

    j.startArray("managed_loggers");


    for (auto c : m_configurations) {
        std::string loggerId = c.first;
        if (m_remoteManagedLoggers.find(loggerId) != m_remoteManagedLoggers.end()) {
            // do not save known loggers fetched by URL
            continue;
        }
        j.startObject();
        j.addValue("logger_id", loggerId);
        j.addValue("configuration_file", c.second);

        std::string frequencyStr;
        if (m_rotationFrequencies.find(loggerId) != m_rotationFrequencies.end()
                && m_rotationFrequencies.at(loggerId) != Configuration::RotationFrequency::NEVER) {
            switch (m_rotationFrequencies.at(loggerId)) {
            case HOURLY:
                frequencyStr = "HOURLY";
                break;
            case DAILY:
                frequencyStr = "DAILY";
                break;
            case WEEKLY:
                frequencyStr = "WEEKLY";
                break;
            case MONTHLY:
                frequencyStr = "MONTHLY";
                break;
            case YEARLY:
                frequencyStr = "YEARLY";
                break;
            default:
                frequencyStr = "NEVER";
            }
            j.addValue("rotation_freq", frequencyStr);
        }

        if (m_archivedLogsFilenames.find(loggerId) != m_archivedLogsFilenames.end()) {
            j.addValue("archived_log_filename", m_archivedLogsFilenames.at(loggerId).substr(std::string("%logger-").size()));
        }

        if (m_archivedLogsCompressedFilenames.find(loggerId) != m_archivedLogsCompressedFilenames.end()) {
            j.addValue("archived_log_compressed_filename", m_archivedLogsCompressedFilenames.at(loggerId));
        }

        if (m_archivedLogsDirectories.find(loggerId) != m_archivedLogsDirectories.end()) {
            j.addValue("archived_log_directory", m_archivedLogsDirectories.at(loggerId));
        }

        if (m_managedLoggerUserMap.find(loggerId) != m_managedLoggerUserMap.end()) {
            j.addValue("user", m_managedLoggerUserMap.at(loggerId));
        }

        j.endObject();
    }
    j.endArray(); // managed_loggers
    if (!m_managedLoggersEndpoint.empty()) {
        j.addValue("managed_loggers_endpoint", m_managedLoggersEndpoint);
    }

    j.startArray("extensions");

    auto createObjectForExtension = [&](const std::vector<Extension*>& list) {
        for (auto& e : list) {
            j.startObject();
            j.addValue("name", e->m_id);
            j.addValue("module", e->m_modulePath);
            j.addValue("description", e->m_description);
            std::string cfg = e->m_config.dump();
            if (cfg.size() > 2 /* i.e, not {} */ && cfg != "null") {
                j.addValue("config", "-==-" + cfg + "<-==-");
            }
            j.endObject();
        }
    };

    createObjectForExtension(m_logExtensions);
    createObjectForExtension(m_preArchiveExtensions);
    createObjectForExtension(m_postArchiveExtensions);
    createObjectForExtension(m_dispatchErrorExtensions);

    j.endArray(); // extensions

    j.endObject(); // end of root

    // workaround for extension config
    std::string src(source);
    Utils::replaceAll(src, "\"config\":\"-==-", "\"config\":");
    Utils::replaceAll(src, "<-==-\"", "");

    JsonDoc jdoc(src);

    if (!jdoc.isValid()) {
        return "Failed to validate exported config";
    }
    return jdoc.dump(4);
}
