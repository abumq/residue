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

#include <sys/stat.h>
#include <pwd.h>
#include <set>
#include <fstream>
#include <utility>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "logging/log.h"
#include "core/configuration.h"
#include "crypto/aes.h"
#include "crypto/base16.h"
#include "crypto/rsa.h"
#include "utils/utils.h"
#include "net/http-client.h"
#include "core/residue-exception.h"
#include "logging/user-log-builder.h"
#include "logging/log-request.h"
#include "core/json-doc.h"
#include "core/json-builder.h"

using namespace residue;

const std::string Configuration::DEFAULT_ACCESS_CODE = "default";
const int Configuration::MAX_BLACKLIST_LOGGERS = 10000;

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
    m_accessCodeBlacklist.clear();
    m_accessCodes.clear();
    m_loggerFlags.clear();
    m_blacklist.clear();
    m_knownClientsEndpoint.clear();
    m_knownClientsKeys.clear();
    m_knownClientsLoggers.clear();
    m_knownLoggersEndpoint.clear();
    m_remoteKnownClients.clear();
    m_remoteKnownLoggers.clear();
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
    m_knownLoggerUserMap.clear();
    m_knownClientDefaultLogger.clear();
    m_logExtensions.clear();
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
    m_tokenPort = m_jsonDoc.get<int>("token_port", 8778);
    m_loggingPort = m_jsonDoc.get<int>("logging_port", 8779);
    m_isValid = m_adminPort > 0 && m_connectPort > 0 && m_loggingPort > 0 && m_tokenPort > 0;


    if (!m_isValid) {
        errorStream  << "  Invalid port(s). Please choose all 4 valid ports." << std::endl;
    }

    if (m_jsonDoc.get<bool>("enable_cli", true)) {
        addFlag(Configuration::Flag::ENABLE_CLI);
    }
    if (m_jsonDoc.get<bool>("allow_unknown_loggers", true)) {
        addFlag(Configuration::Flag::ALLOW_UNKNOWN_LOGGERS);
    }
    if (m_jsonDoc.get<bool>("allow_insecure_connection", true)) {
        addFlag(Configuration::ALLOW_INSECURE_CONNECTION);
    }
    if (m_jsonDoc.get<bool>("compression", true)) {
        addFlag(Configuration::COMPRESSION);
    }
    if (m_jsonDoc.get<bool>("allow_unknown_clients", true)) {
        addFlag(Configuration::Flag::ALLOW_UNKNOWN_CLIENTS);
    }
    if (m_jsonDoc.get<bool>("requires_token", true)) {
        addFlag(Configuration::Flag::REQUIRES_TOKEN);
    }
    if (m_jsonDoc.get<bool>("allow_default_access_code", false)) {
        addFlag(Configuration::Flag::ALLOW_DEFAULT_ACCESS_CODE);
    }
    if (m_jsonDoc.get<bool>("allow_bulk_log_request", true)) {
        addFlag(Configuration::Flag::ALLOW_BULK_LOG_REQUEST);
    }
    if (m_jsonDoc.get<bool>("immediate_flush", true)) {
        addFlag(Configuration::Flag::IMMEDIATE_FLUSH);
    }
    if (m_jsonDoc.get<bool>("requires_timestamp", true)) {
        addFlag(Configuration::Flag::REQUIRES_TIMESTAMP);
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
            Utils::resolveResidueHomeEnvVar(m_serverRSAPrivateKeyFile);
            Utils::resolveResidueHomeEnvVar(m_serverRSAPublicKeyFile);

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


                if (!RSA::verifyKeyPair(RSA::loadPrivateKey(rsaPrivateKey, rsaPrivateKeySecret), RSA::loadPublicKey(rsaPublicKey), rsaPrivateKeySecret)) {
                    errorStream << "  Verify server key: Invalid RSA key pair.";
                } else {
                    m_serverRSAKey.privateKey = RSA::loadPrivateKey(rsaPrivateKey, rsaPrivateKeySecret);
                    m_serverRSAKey.publicKey = RSA::loadPublicKey(rsaPublicKey);
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
    } else if (!hasFileMode(static_cast<unsigned int>(S_IRUSR)) && !hasFileMode(static_cast<unsigned int>(S_IRGRP))) {
        errorStream << "  File mode invalid [" << m_fileMode << "]. Either user or group should be able to read the log files" << std::endl;
    }
    m_nonAcknowledgedClientAge = m_jsonDoc.get<unsigned int>("non_acknowledged_client_age", 300);
    if (m_nonAcknowledgedClientAge < 120) {
        RLOG(WARNING) << "Invalid value for [non_acknowledged_client_age]. Setting it to default [120]";
        m_nonAcknowledgedClientAge = 120;
    }
    m_timestampValidity = m_jsonDoc.get<unsigned int>("timestamp_validity", 120);
    if (m_timestampValidity < 30) {
        RLOG(WARNING) << "Invalid value for [timestamp_validity]. Setting it to minimum [30]";
        m_timestampValidity = 30;
    }
    m_maxTokenAge = m_jsonDoc.get<unsigned int>("max_token_age", 0);
    if (m_maxTokenAge != 0 && m_maxTokenAge < 15) {
        RLOG(WARNING) << "Invalid value for [max_token_age]. Setting it to minimum [15]";
        m_maxTokenAge = 15;
    }
    bool hasTokenAgeLimit = m_maxTokenAge > 0;

    m_tokenAge = m_jsonDoc.get<unsigned int>("token_age", std::min(3600U, m_maxTokenAge));
    if (m_tokenAge == 0 && hasTokenAgeLimit) {
        errorStream << "Cannot set token age [token_age] to 'forever' as [max_token_age] is " << m_maxTokenAge;
    } else if (m_tokenAge > m_maxTokenAge && hasTokenAgeLimit) {
        errorStream << "Cannot set token age [token_age] greater than [max_token_age] which is " << m_maxTokenAge;
    } else if (m_tokenAge != 0 && m_tokenAge < 15) {
        RLOG(WARNING) << "Invalid value for [token_age]. Setting it to minimum [15]";
        m_tokenAge = 15;
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


    // We load known loggers before known clients because
    // known clients may have "loggers" array
    // that will be cross-checked with loggers list

    if (m_jsonDoc.hasKey("known_loggers")) {
        loadKnownLoggers(m_jsonDoc.get<JsonDoc::Value>("known_loggers", JsonDoc::Value()), errorStream, false);
    }

    auto queryEndpoint = [&](const std::string& endpoint,
            const std::string& keyName,
            const std::function<void(const JsonDoc::Value&)>& cb) {

        RVLOG(RV_INFO) << "Querying [" << endpoint << "]...";
        std::string contents;

        try {
            contents = HttpClient::fetchUrlContents(endpoint);
            Utils::trim(contents);
            if (!contents.empty()) {
                JsonDoc j(contents);
                if (j.isValid()) {
                    if (j.hasKey(keyName.c_str())) {
                        cb(j.get<JsonDoc::Value>(keyName.c_str(), JsonDoc::Value()));
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

    if (m_jsonDoc.hasKey("known_loggers_endpoint")) {
        m_knownLoggersEndpoint = m_jsonDoc.get<std::string>("known_loggers_endpoint", "");
        if (!m_knownLoggersEndpoint.empty()) {
            queryEndpoint(m_knownLoggersEndpoint, "known_loggers", [&](const JsonDoc::Value& json) {
                loadKnownLoggers(json, errorStream, true);
            });
        }
    }

    if (m_jsonDoc.hasKey("known_clients")) {
        loadKnownClients(m_jsonDoc.get<JsonDoc::Value>("known_clients", JsonDoc::Value()), errorStream, false);
    }

    if (m_jsonDoc.hasKey("known_clients_endpoint")) {
        m_knownClientsEndpoint = m_jsonDoc.get<std::string>("known_clients_endpoint", "");
        if (!m_knownClientsEndpoint.empty()) {
            queryEndpoint(m_knownClientsEndpoint, "known_clients", [&](const JsonDoc::Value& json) {
                loadKnownClients(json, errorStream, true);
            });
        }
    }

    JsonDoc::Value jLoggersBlacklist = m_jsonDoc.get<JsonDoc::Value>("loggers_blacklist", JsonDoc::Value());
    if (jLoggersBlacklist.isArray()) {
        loadLoggersBlacklist(jLoggersBlacklist, errorStream);
    }


 #ifdef RESIDUE_HAS_EXTENSIONS
    JsonDoc::Value jExtensionsVal = m_jsonDoc.get<JsonDoc::Value>("extensions", JsonDoc::Value());
    if (jExtensionsVal.isObject()) {
        JsonDoc jExtensions(jExtensionsVal);
        JsonDoc::Value jLogExtensions = jExtensions.get<JsonDoc::Value>("log_extensions", JsonDoc::Value());
        if (jLogExtensions.isArray()) {
            loadExtensions<LogExtension>(jLogExtensions, errorStream, &m_logExtensions);
        }
    }
 #endif

 #ifndef RESIDUE_HAS_CURL
    if (!m_knownClientsEndpoint.empty() || !m_knownLoggersEndpoint.empty()) {
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


void Configuration::loadKnownLoggers(const JsonDoc::Value& json, std::stringstream& errorStream, bool viaUrl)
{
    for (const auto& logger : json) {
        JsonDoc j(logger);

        std::string loggerId = j.get<std::string>("logger_id", "");
        if (loggerId.empty()) {
            errorStream << "  Logger ID not provided in known_loggers" << std::endl;
            continue;
        }
        if (m_configurations.find(loggerId) != m_configurations.end()) {
            errorStream << "  Duplicate logger in known_loggers [" << loggerId << "]" << std::endl;
            continue;
        }
        std::string easyloggingConfigFile = j.get<std::string>("configuration_file", "");
        if (!easyloggingConfigFile.empty()) {
            Utils::resolveResidueHomeEnvVar(easyloggingConfigFile);
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
                m_remoteKnownLoggers.insert(loggerId);
            }

            // load logger and configure
            el::Configurations confs(easyloggingConfigFile);
            el::Logger* logger = el::Loggers::getLogger(loggerId);
            el::base::type::EnumType lIndex = el::LevelHelper::kMinValid;
            el::Loggers::reconfigureLogger(logger, confs);
            std::string fn = logger->typedConfigurations()->filename(el::Level::Info);
            std::string fn2 = logger->typedConfigurations()->filename(el::Level::Info);
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

        std::string loggerUser = j.get<std::string>("user", "");
        if (!loggerUser.empty()) {
            struct passwd* userpwd = getpwnam(loggerUser.data());
            if (userpwd == nullptr) {
                errorStream << "  User corresponding to logger [" << loggerId << "] does not exist [" << loggerUser << "]" << std::endl;
                endpwent();
                continue;
            }
            endpwent();
            m_knownLoggerUserMap.insert(std::make_pair(loggerId, loggerUser));
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

        JsonDoc::Value accessCodes = j.get<JsonDoc::Value>("access_codes", JsonDoc::Value());
        if (accessCodes.isArray()) {
            for (const auto& accessCode : accessCodes) {
                JsonDoc jAccessCode(accessCode);
                std::string code = jAccessCode.get<std::string>("code", "");
                if (code.empty() || code == DEFAULT_ACCESS_CODE) {
                    continue;
                }
                unsigned int age = jAccessCode.get<unsigned int>("token_age", tokenAge());

                if (m_maxTokenAge > 0 && age > m_maxTokenAge) {
                    errorStream << "Cannot set token age for logger [" << loggerId << "], access code ["
                                << code << "] greater than [max_token_age] which is " << m_maxTokenAge;
                } else if (m_maxTokenAge > 0 && age == 0) {
                    errorStream << "Cannot set token age for logger [" << loggerId << "], access code ["
                                << code << "] to 'forever' [max_token_age] is " << m_maxTokenAge;
                }

                const auto& it = m_accessCodes.find(loggerId);
                if (it == m_accessCodes.end()) {
                    std::unordered_set<AccessCode> singleCodeSet = { AccessCode(code, age) };
                    m_accessCodes[loggerId] = singleCodeSet;
                } else {
                    it->second.insert(AccessCode(code, age));
                }
            }
        }

        JsonDoc::Value accessCodeBlacklist = j.get<JsonDoc::Value>("access_code_blacklist", JsonDoc::Value());
        if (accessCodeBlacklist.isArray()) {
            JsonDoc jAccessCode(accessCodeBlacklist);

            for (const auto& accessCode : jAccessCode) {
                JsonDoc ja(accessCode);

                std::string code = ja.as<std::string>("");
                if (code.empty() || code == DEFAULT_ACCESS_CODE) {
                    continue;
                }

                if (isValidAccessCode(loggerId, code)) {
                    errorStream << "  Access code [" << code << "] exists in both allowed and blacklist lists" << std::endl;
                    continue;
                }
                const auto& it = m_accessCodeBlacklist.find(loggerId);
                if (it == m_accessCodeBlacklist.end()) {
                    std::unordered_set<std::string> singleCodeSet = {code};
                    m_accessCodeBlacklist[loggerId] = singleCodeSet;
                } else {
                    if (std::find(it->second.begin(), it->second.end(), code) == it->second.end()) {
                        it->second.insert(code);
                    }
                }
            }
        }
    }
}

void Configuration::loadKnownClients(const JsonDoc::Value& json, std::stringstream& errorStream, bool viaUrl)
{
    for (const auto& knownClientPair : json) {
        JsonDoc j(knownClientPair);
        std::string clientId = j.get<std::string>("client_id", "");
        if (clientId.empty()) {
            errorStream << "  Client ID not provided in known_clients" << std::endl;
            continue;
        }
        if (!Utils::isAlphaNumeric(clientId, "-_@#")) {
            errorStream << "  Invalid character in client ID, should be alpha-numeric (can also include these characters excluding square brackets: [_@-#])" << std::endl;
            continue;
        }
        std::string publicKey = j.get<std::string>("public_key", "");
        if (publicKey.empty()) {
            errorStream << "  RSA public key not provided in known_clients for [" << clientId << "]" << std::endl;
            continue;
        }
        Utils::resolveResidueHomeEnvVar(publicKey);
        if (m_knownClientsKeys.find(clientId) != m_knownClientsKeys.end()) {
            errorStream << "  Duplicate client ID in known_clients [" << clientId << "]" << std::endl;
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

        m_knownClientsKeys.insert(std::make_pair(clientId, std::make_pair(publicKey, publicKeyContents)));

        if (viaUrl) {
            m_remoteKnownClients.insert(clientId);
        }

        unsigned int keySize = j.get<unsigned int>("key_size", 0);

        if (keySize == 128 || keySize == 192 || keySize == 256) {
            m_keySizes.insert(std::make_pair(clientId, keySize));
        } else {
            if (keySize != 0) {
                errorStream << "  Invalid key size [" << keySize << "] for client [" << clientId << "]. Please choose 128, 192 or 256-bit" << std::endl;
            }
        }

        JsonDoc::Value loggers = j.get<JsonDoc::Value>("loggers", JsonDoc::Value());

        if (loggers.isArray()) {
            std::unordered_set<std::string> loggerIds;
            for (const auto& loggerNode : loggers) {
                JsonDoc jLoggers(loggerNode);
                std::string loggerId = jLoggers.as<std::string>("");
                if (loggerId.empty()) {
                    continue;
                }
                if (!isKnownLogger(loggerId)) {
                    errorStream << "  Logger [" << loggerId << "] for client [" << clientId << "] is unknown" << std::endl;
                    continue;
                }
                loggerIds.insert(loggerId);
            }
            m_knownClientsLoggers.insert(std::make_pair(clientId, loggerIds));

            std::string defaultLogger = j.get<std::string>("default_logger", "");
            if (!defaultLogger.empty()) {
                if (loggerIds.find(defaultLogger) != loggerIds.end()) {
                    m_knownClientDefaultLogger.insert(std::make_pair(clientId, defaultLogger));
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
                    if (m_knownLoggerUserMap.find(loggerId) == m_knownLoggerUserMap.end()) {
                        m_knownLoggerUserMap.insert(std::make_pair(loggerId, loggerUser));
                    } else {
                        // for same user ignore, for different user this is a config warning
                        std::string existingAssignedUser = m_knownLoggerUserMap.at(loggerId);
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
                errorStream << "  Default logger ["  << defaultLogger << "] for client [" << clientId << "] is not part of [loggers] array. Please see https://github.com/muflihun/residue/blob/master/docs/CONFIGURATION.md#known_clientsloggers";
            }
        }
    }
}

void Configuration::loadLoggersBlacklist(const JsonDoc::Value& json, std::stringstream& errorStream)
{
    for (const auto& loggerId : json) {
        JsonDoc j(loggerId);
        std::string loggerIdStr = j.as<std::string>("");
        if (loggerIdStr.empty()) {
            continue;
        }
        if (isKnownLogger(loggerIdStr)) {
            errorStream << "  Cannot blacklist [" << loggerId << "] logger. Remove it from 'known_loggers' first." << std::endl;
            continue;
        }
        if (m_blacklist.size() >= MAX_BLACKLIST_LOGGERS) {
            errorStream << "  You have added maximum number of blacklisted loggers. Please consider using 'allow_unknown_loggers' instead." << std::endl;
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

bool Configuration::addKnownClient(const std::string& clientId,
                                   const std::string& publicKey)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_knownClientsKeys.find(clientId) != m_knownClientsKeys.end()) {
        RLOG(ERROR) << "Known client already exists";
        return false;
    }

    std::ifstream fs(publicKey, std::ios::in);
    if (!fs.is_open()) {
        RLOG(ERROR) << "Public key file not readable";
        return false;
    }
    std::string publicKeyContents = std::string(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());

    m_knownClientsKeys.insert(std::make_pair(clientId, std::make_pair(publicKey, publicKeyContents)));
    return true;
}

void Configuration::removeKnownClient(const std::string& clientId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto pos = m_knownClientsKeys.find(clientId);
    if (pos != m_knownClientsKeys.end()) {
        m_knownClientsKeys.erase(pos);
    }
}

bool Configuration::verifyKnownClient(const std::string& clientId, const std::string& signature) const
{
    if (!isKnownClient(clientId)) {
        return false;
    }
    RSA::PublicKey rsaPublicKey = RSA::loadPublicKey(m_knownClientsKeys.at(clientId).second); // second = public key contents
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

bool Configuration::isValidAccessCode(const std::string& loggerId,
                                      const std::string& accessCode) const
{
    if ((accessCode.empty() || accessCode == DEFAULT_ACCESS_CODE) && hasFlag(Configuration::Flag::ALLOW_DEFAULT_ACCESS_CODE)) {
        return true;
    }
    const auto& blacklistIter = m_accessCodeBlacklist.find(loggerId);
    if (blacklistIter != m_accessCodeBlacklist.end()) {
        const std::unordered_set<std::string>* blackListAccessCodes = &(blacklistIter->second);
        if (std::find(blackListAccessCodes->begin(), blackListAccessCodes->end(), accessCode) != blackListAccessCodes->end()) {
            // Current access code used is blacklisted
            RVLOG(RV_DEBUG) << "Access code blacklisted [" << accessCode << "]";
            return false;
        }
    }
    const auto& iter = m_accessCodes.find(loggerId);
    if (iter == m_accessCodes.end()) {
        RVLOG(RV_DEBUG) << "Access code not valid [" << accessCode << "]";
        return false;
    }
    return std::find(iter->second.begin(), iter->second.end(), accessCode) != iter->second.end();
}

#if 0
std::string Configuration::getConfigurationFile(const std::string& loggerId, const UserLogBuilder* userLogBuilder) const
{
    return getConfigurationFile(loggerId, userLogBuilder != nullptr ? userLogBuilder->request() : nullptr);
}
#endif

std::string Configuration::getConfigurationFile(const std::string& loggerId, const LogRequest* request) const
{
    DRVLOG(RV_DEBUG) << "Finding configuration file for [" << loggerId << "]";
    if (m_configurations.find(loggerId) != m_configurations.end()) {
        DRVLOG(RV_DEBUG) << "Configuration file found for [" << loggerId << "]";
        return m_configurations.at(loggerId);
    }
    // get conf of client's default logger
    if (request != nullptr) {
        DRVLOG(RV_DEBUG) << "Finding configuration file using client ID [" << request->clientId() << "]";
        if (m_knownClientDefaultLogger.find(request->clientId()) != m_knownClientDefaultLogger.end()) {
            std::string defaultLoggerForClient = m_knownClientDefaultLogger.at(request->clientId());
            if (m_configurations.find(defaultLoggerForClient) != m_configurations.end()) {
                DRVLOG(RV_DEBUG) << "Found configuration file for logger [" << loggerId << "] via default logger ["
                                 << defaultLoggerForClient << "] for client [" << request->clientId() << "]";
                return m_configurations.at(defaultLoggerForClient);
            }
        }
    }
    return "";
}

void Configuration::updateUnknownLoggerUserFromRequest(const std::string& loggerId, const LogRequest* request)
{
    if (m_unknownLoggerUserMap.find(loggerId) != m_unknownLoggerUserMap.end()) {
        return; // already set
    }
    // get conf of client's default logger
    if (request != nullptr) {
        RVLOG(RV_INFO) << "Updating user for unknown logger [" << loggerId << "] using client [" << request->clientId() << "]";
        if (m_knownClientDefaultLogger.find(request->clientId()) != m_knownClientDefaultLogger.end()) {
            std::string defaultLoggerForClient = m_knownClientDefaultLogger.at(request->clientId());
            std::string user = findLoggerUser(defaultLoggerForClient);
            RVLOG(RV_INFO) << "Found user for unknown logger [" << loggerId << "] => [" << user << "]";
            m_unknownLoggerUserMap.insert(std::make_pair(loggerId, user));
        }
    }
}

int Configuration::getAccessCodeTokenLife(const std::string& loggerId,
                                          const std::string& accessCode) const
{
    if ((accessCode.empty() || accessCode == DEFAULT_ACCESS_CODE) && hasFlag(Configuration::Flag::ALLOW_DEFAULT_ACCESS_CODE)) {
        return m_tokenAge;
    }
    const auto& iter = m_accessCodes.find(loggerId);
    if (iter == m_accessCodes.end()) {
        return -1;
    }
    const auto& acIter = std::find(iter->second.begin(), iter->second.end(), accessCode);
    if (acIter == iter->second.end()) {
        return -1;
    }
    return acIter->age();
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
    if (m_knownLoggerUserMap.find(loggerId) != m_knownLoggerUserMap.end()) {
        return m_knownLoggerUserMap.at(loggerId);
    }
    if (m_unknownLoggerUserMap.find(loggerId) != m_unknownLoggerUserMap.end()) {
        return m_unknownLoggerUserMap.at(loggerId);
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
