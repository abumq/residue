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
#include "core/json-document.h"
#include "core/json-doc.h"
#include "crypto/aes.h"
#include "crypto/base16.h"
#include "crypto/rsa.h"
#include "utils/utils.h"
#include "net/http-client.h"
#include "core/residue-exception.h"
#include "logging/user-log-builder.h"
#include "logging/log-request.h"

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
    m_accessCodesBlacklist.clear();
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
#ifdef RESIDUE_USE_GASON
    JsonDoc jdoc(jsonStr);
    if (!jdoc.isValid()) {
        m_isMalformedJson = true;
        m_isValid = false;
        m_errors = jdoc.errorText();
        return;
    }
    m_adminPort = jdoc.get<int>("admin_port", 8776);
    m_connectPort = jdoc.get<int>("connect_port", 8777);
    m_tokenPort = jdoc.get<int>("token_port", 8778);
    m_loggingPort = jdoc.get<int>("logging_port", 8779);
    m_isValid = m_adminPort > 0 && m_connectPort > 0 && m_loggingPort > 0 && m_tokenPort > 0;


    if (!m_isValid) {
        errorStream  << "  Invalid port(s). Please choose all 4 valid ports." << std::endl;
    }

    if (jdoc.get<bool>("accept_input", true)) {
        addFlag(Configuration::Flag::ACCEPT_INPUT);
    }
    if (jdoc.get<bool>("allow_unknown_loggers", true)) {
        addFlag(Configuration::Flag::ALLOW_UNKNOWN_LOGGERS);
    }
    if (jdoc.get<bool>("allow_plain_connection", true)) {
        addFlag(Configuration::ALLOW_PLAIN_CONNECTION);
    }
    if (jdoc.get<bool>("compression", true)) {
        addFlag(Configuration::COMPRESSION);
    }
    if (jdoc.get<bool>("allow_unknown_clients", true)) {
        addFlag(Configuration::Flag::ALLOW_UNKNOWN_CLIENTS);
    }
    if (jdoc.get<bool>("requires_token", true)) {
        addFlag(Configuration::Flag::REQUIRES_TOKEN);
    }
    if (jdoc.get<bool>("allow_default_access_code", false)) {
        addFlag(Configuration::Flag::ALLOW_DEFAULT_ACCESS_CODE);
    }
    if (jdoc.get<bool>("allow_plain_log_request", false)) {
        addFlag(Configuration::Flag::ALLOW_PLAIN_LOG_REQUEST);
    }
    if (jdoc.get<bool>("allow_bulk_log_request", true)) {
        addFlag(Configuration::Flag::ALLOW_BULK_LOG_REQUEST);
    }
    if (jdoc.get<bool>("immediate_flush", true)) {
        addFlag(Configuration::Flag::IMMEDIATE_FLUSH);
    }
    if (jdoc.get<bool>("requires_timestamp", false)) {
        addFlag(Configuration::Flag::REQUIRES_TIMESTAMP);
    }

    m_serverKey = jdoc.get<std::string>("server_key", AES::generateKey(256));

    if (m_serverKey.size() != 64) {
        errorStream << "  Invalid value for [server_key]. It should be hex-value of 256-bit key. "
                    << "e.g, 048CB7050312DB329788CE1533C294A1F248F8A1BD6F611D7516803EDE271C65. "
                    << "Use mine to generate a key: [mine -g --aes 256]" << std::endl;
    }

    m_serverRSAPrivateKeyFile = jdoc.get<std::string>("server_rsa_private_key", "");
    std::string rsaPrivateKeySecret = Base16::decode(jdoc.get<std::string>("server_rsa_secret", ""));
    if (!m_serverRSAPrivateKeyFile.empty()) {

        m_serverRSAPublicKeyFile = jdoc.get<std::string>("server_rsa_public_key", "");
        if (m_serverRSAPublicKeyFile.empty()) {
            errorStream << "  Both RSA private and public keys must be provided if provided at all" << std::endl;
        } else {
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
    } else if (!hasFlag(Configuration::ALLOW_PLAIN_CONNECTION)) {
        errorStream << "  Server does not allow plain connections. Please provide RSA key pair" << std::endl;
    }

    m_archivedLogDirectory = jdoc.get<std::string>("archived_log_directory", "");
    if (m_archivedLogDirectory.empty()) {
        errorStream << "  Please choose valid default archived_log_directory" << std::endl;
    }

    m_archivedLogFilename = jdoc.get<std::string>("archived_log_filename", "");
    if (m_archivedLogFilename.empty()) {
        errorStream << "  Please choose valid default archived_log_filename" << std::endl;
    }

    m_archivedLogCompressedFilename = jdoc.get<std::string>("archived_log_compressed_filename", "");
    if (m_archivedLogCompressedFilename.empty() || m_archivedLogCompressedFilename == m_archivedLogFilename
            || m_archivedLogCompressedFilename.find("/") != std::string::npos
            || m_archivedLogCompressedFilename.find("\\") != std::string::npos) {
        errorStream << "  Please choose valid default archived_log_compressed_filename" << std::endl;
    }

    m_clientAge = jdoc.get<unsigned int>("client_age", 259200);
    if (m_clientAge != 0 && m_clientAge < 120) {
        RLOG(WARNING) << "Invalid value for [client_age]. Setting it to minimum [120]";
        m_clientAge = 120;
    }
    m_defaultKeySize = jdoc.get<unsigned int>("default_key_size", 256);
    if (m_defaultKeySize != 128 && m_defaultKeySize != 192 && m_defaultKeySize != 256) {
        errorStream << "  Invalid default key size. Please choose 128, 192 or 256-bit" << std::endl;
    }

    auto hasFileMode = [&](unsigned int mode) {
        return m_fileMode != 0 && (m_fileMode & mode) != 0;
    };

    m_fileMode = jdoc.get<unsigned int>("file_mode", static_cast<unsigned int>(S_IRUSR | S_IWUSR | S_IRGRP));
    if (hasFileMode(static_cast<unsigned int>(S_IWOTH)) || hasFileMode(static_cast<unsigned int>(S_IXOTH))) {
        errorStream << "  File mode too open [" << m_fileMode << "]. You should at least not allow others to write to the file." << std::endl;
    } else if (!hasFileMode(static_cast<unsigned int>(S_IRUSR)) && !hasFileMode(static_cast<unsigned int>(S_IRGRP))) {
        errorStream << "  File mode invalid [" << m_fileMode << "]. Either user or group should be able to read the log files" << std::endl;
    }
    m_nonAcknowledgedClientAge = jdoc.get<unsigned int>("non_acknowledged_client_age", 300);
    if (m_nonAcknowledgedClientAge < 120) {
        RLOG(WARNING) << "Invalid value for [non_acknowledged_client_age]. Setting it to default [120]";
        m_nonAcknowledgedClientAge = 120;
    }
    m_timestampValidity = jdoc.get<unsigned int>("timestamp_validity", 120);
    if (m_timestampValidity < 30) {
        RLOG(WARNING) << "Invalid value for [timestamp_validity]. Setting it to minimum [30]";
        m_timestampValidity = 30;
    }
    m_maxTokenAge = jdoc.get<unsigned int>("max_token_age", 0);
    if (m_maxTokenAge != 0 && m_maxTokenAge < 15) {
        RLOG(WARNING) << "Invalid value for [max_token_age]. Setting it to minimum [15]";
        m_maxTokenAge = 15;
    }
    bool hasTokenAgeLimit = m_maxTokenAge > 0;

    m_tokenAge = jdoc.get<unsigned int>("token_age", std::min(3600U, m_maxTokenAge));
    if (m_tokenAge == 0 && hasTokenAgeLimit) {
        errorStream << "Cannot set token age [token_age] to 'forever' as [max_token_age] is " << m_maxTokenAge;
    } else if (m_tokenAge > m_maxTokenAge && hasTokenAgeLimit) {
        errorStream << "Cannot set token age [token_age] greater than [max_token_age] which is " << m_maxTokenAge;
    } else if (m_tokenAge != 0 && m_tokenAge < 15) {
        RLOG(WARNING) << "Invalid value for [token_age]. Setting it to minimum [15]";
        m_tokenAge = 15;
    }

    unsigned int defaultClientIntegrityTaskInterval = std::max(300U, std::min(m_clientAge, m_nonAcknowledgedClientAge));
    m_clientIntegrityTaskInterval = jdoc.get<unsigned int>("client_integrity_task_interval", defaultClientIntegrityTaskInterval);
    if (m_clientIntegrityTaskInterval == 0 || m_clientIntegrityTaskInterval < std::min(m_clientAge, m_nonAcknowledgedClientAge)) {
        RLOG(WARNING) << "Invalid value for [client_integrity_task_interval (" << m_clientIntegrityTaskInterval << ")]. "
                      << "Choose anything greater than or equal to " << std::min(m_clientAge, m_nonAcknowledgedClientAge) << " but not zero. Setting it to lower ["
                      << std::min(m_clientAge, m_nonAcknowledgedClientAge) << "]";
        m_clientIntegrityTaskInterval = defaultClientIntegrityTaskInterval;
    }
    m_dispatchDelay = jdoc.get<unsigned int>("dispatch_delay", 1);
    if (m_dispatchDelay > 500) {
        RLOG(WARNING) << "Invalid value for [dispatch_delay]. Setting it to default [1ms]";
        m_dispatchDelay = 1;
    }
    m_maxItemsInBulk = jdoc.get<unsigned int>("max_items_in_bulk", 5);
    if (m_maxItemsInBulk <= 1 || m_maxItemsInBulk >= 100) {
        errorStream << "  Invalid value for [max_items_in_bulk]. Please choose between 2-100" << std::endl;
    }


    // We load known loggers before known clients because
    // known clients may have "loggers" array
    // that will be cross-checked with loggers list

    if (jdoc.hasKey("known_loggers")) {
        loadKnownLoggers(jdoc.get<JsonDoc::Value>("known_loggers", JsonDoc::Value()), errorStream, false);
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

    if (jdoc.hasKey("known_loggers_endpoint")) {
        m_knownLoggersEndpoint = jdoc.get<std::string>("known_loggers_endpoint", "");
        if (!m_knownLoggersEndpoint.empty()) {
            queryEndpoint(m_knownLoggersEndpoint, "known_loggers", [&](const JsonDoc::Value& json) {
                loadKnownLoggers(json, errorStream, true);
            });
        }
    }

    if (jdoc.hasKey("known_clients")) {
        loadKnownClients(jdoc.get<JsonDoc::Value>("known_clients", JsonDoc::Value()), errorStream, false);
    }

    if (jdoc.hasKey("known_clients_endpoint")) {
        m_knownClientsEndpoint = jdoc.get<std::string>("known_clients_endpoint", "");
        if (!m_knownClientsEndpoint.empty()) {
            queryEndpoint(m_knownClientsEndpoint, "known_clients", [&](const JsonDoc::Value& json) {
                loadKnownClients(json, errorStream, true);
            });
        }
    }

    JsonDoc::Value jLoggersBlacklist = jdoc.get<JsonDoc::Value>("loggers_blacklist", JsonDoc::Value());
    if (jLoggersBlacklist.isArray()) {
        loadLoggersBlacklist(jLoggersBlacklist, errorStream);
    }


#ifdef RESIDUE_HAS_EXTENSIONS
    JsonDoc::Value jExtensionsVal = jdoc.get<JsonDoc::Value>("extensions", JsonDoc::Value());
    if (jExtensionsVal.isObject()) {
        JsonDoc jExtensions(jExtensionsVal);
        JsonDoc::Value jLogExtensions = jExtensions.get<JsonDoc::Value>("log_extensions", JsonDoc::Value());
        if (jLogExtensions.isArray()) {
            loadExtensions<LogExtension>(jLogExtensions, errorStream, &m_logExtensions);
        }
    }
#endif

#else

    JsonDocument jsonDoc(std::move(jsonStr));
    if (!jsonDoc.isValid()) {
        m_isMalformedJson = true;
        m_isValid = false;
        m_errors = jsonDoc.lastError();
        return;
    }
    JsonItem root = jsonDoc.root();

    m_adminPort = jsonDoc.getUInt("admin_port", 8776);
    m_connectPort = jsonDoc.getUInt("connect_port", 8777);
    m_tokenPort = jsonDoc.getUInt("token_port", 8778);
    m_loggingPort = jsonDoc.getUInt("logging_port", 8779);
    m_isValid = m_adminPort > 0 && m_connectPort > 0 && m_loggingPort > 0 && m_tokenPort > 0;

    if (!m_isValid) {
        errorStream  << "  Invalid port(s). Please choose all 4 valid ports." << std::endl;
    }

    if (jsonDoc.getBool("accept_input", true)) {
        addFlag(Configuration::Flag::ACCEPT_INPUT);
    }
    if (jsonDoc.getBool("allow_unknown_loggers", true)) {
        addFlag(Configuration::Flag::ALLOW_UNKNOWN_LOGGERS);
    }
    if (jsonDoc.getBool("allow_plain_connection", true)) {
        addFlag(Configuration::ALLOW_PLAIN_CONNECTION);
    }
    if (jsonDoc.getBool("compression", true)) {
        addFlag(Configuration::COMPRESSION);
    }
    if (jsonDoc.getBool("allow_unknown_clients", true)) {
        addFlag(Configuration::Flag::ALLOW_UNKNOWN_CLIENTS);
    }
    if (jsonDoc.getBool("requires_token", true)) {
        addFlag(Configuration::Flag::REQUIRES_TOKEN);
    }
    if (jsonDoc.getBool("allow_default_access_code", false)) {
        addFlag(Configuration::Flag::ALLOW_DEFAULT_ACCESS_CODE);
    }
    if (jsonDoc.getBool("allow_plain_log_request", false)) {
        addFlag(Configuration::Flag::ALLOW_PLAIN_LOG_REQUEST);
    }
    if (jsonDoc.getBool("allow_bulk_log_request", true)) {
        addFlag(Configuration::Flag::ALLOW_BULK_LOG_REQUEST);
    }
    if (jsonDoc.getBool("immediate_flush", true)) {
        addFlag(Configuration::Flag::IMMEDIATE_FLUSH);
    }
    if (jsonDoc.getBool("requires_timestamp", false)) {
        addFlag(Configuration::Flag::REQUIRES_TIMESTAMP);
    }

    m_serverKey = jsonDoc.getString("server_key", AES::generateKey(256));

    if (m_serverKey.size() != 64) {
        errorStream << "  Invalid value for [server_key]. It should be hex-value of 256-bit key. "
                    << "e.g, 048CB7050312DB329788CE1533C294A1F248F8A1BD6F611D7516803EDE271C65. "
                    << "Use mine to generate a key: [mine -g --aes 256]" << std::endl;
    }

    m_serverRSAPrivateKeyFile = jsonDoc.getString("server_rsa_private_key");
    std::string rsaPrivateKeySecret = Base16::decode(jsonDoc.getString("server_rsa_secret"));
    if (!m_serverRSAPrivateKeyFile.empty()) {

        m_serverRSAPublicKeyFile = jsonDoc.getString("server_rsa_public_key");
        if (m_serverRSAPublicKeyFile.empty()) {
            errorStream << "  Both RSA private and public keys must be provided if provided at all" << std::endl;
        } else {
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
    } else if (!hasFlag(Configuration::ALLOW_PLAIN_CONNECTION)) {
        errorStream << "  Server does not allow plain connections. Please provide RSA key pair" << std::endl;
    }

    m_archivedLogDirectory = jsonDoc.getString("archived_log_directory");
    if (m_archivedLogDirectory.empty()) {
        errorStream << "  Please choose valid default archived_log_directory" << std::endl;
    }

    m_archivedLogFilename = jsonDoc.getString("archived_log_filename");
    if (m_archivedLogFilename.empty()) {
        errorStream << "  Please choose valid default archived_log_filename" << std::endl;
    }

    m_archivedLogCompressedFilename = jsonDoc.getString("archived_log_compressed_filename");
    if (m_archivedLogCompressedFilename.empty() || m_archivedLogCompressedFilename == m_archivedLogFilename
            || m_archivedLogCompressedFilename.find("/") != std::string::npos
            || m_archivedLogCompressedFilename.find("\\") != std::string::npos) {
        errorStream << "  Please choose valid default archived_log_compressed_filename" << std::endl;
    }

    m_clientAge = jsonDoc.getUInt("client_age", 259200);
    if (m_clientAge != 0 && m_clientAge < 120) {
        RLOG(WARNING) << "Invalid value for [client_age]. Setting it to minimum [120]";
        m_clientAge = 120;
    }
    m_defaultKeySize = jsonDoc.getUInt("default_key_size", 256);
    if (m_defaultKeySize != 128 && m_defaultKeySize != 192 && m_defaultKeySize != 256) {
        errorStream << "  Invalid default key size. Please choose 128, 192 or 256-bit" << std::endl;
    }

    auto hasFileMode = [&](unsigned int mode) {
        return m_fileMode != 0 && (m_fileMode & mode) != 0;
    };

    m_fileMode = jsonDoc.getUInt("file_mode", static_cast<unsigned int>(S_IRUSR | S_IWUSR | S_IRGRP));
    if (hasFileMode(static_cast<unsigned int>(S_IWOTH)) || hasFileMode(static_cast<unsigned int>(S_IXOTH))) {
        errorStream << "  File mode too open [" << m_fileMode << "]. You should at least not allow others to write to the file." << std::endl;
    } else if (!hasFileMode(static_cast<unsigned int>(S_IRUSR)) && !hasFileMode(static_cast<unsigned int>(S_IRGRP))) {
        errorStream << "  File mode invalid [" << m_fileMode << "]. Either user or group should be able to read the log files" << std::endl;
    }
    m_nonAcknowledgedClientAge = jsonDoc.getUInt("non_acknowledged_client_age", 300);
    if (m_nonAcknowledgedClientAge < 120) {
        RLOG(WARNING) << "Invalid value for [non_acknowledged_client_age]. Setting it to default [120]";
        m_nonAcknowledgedClientAge = 120;
    }
    m_timestampValidity = jsonDoc.getUInt("timestamp_validity", 120);
    if (m_timestampValidity < 30) {
        RLOG(WARNING) << "Invalid value for [timestamp_validity]. Setting it to minimum [30]";
        m_timestampValidity = 30;
    }
    m_maxTokenAge = jsonDoc.getUInt("max_token_age", 0);
    if (m_maxTokenAge != 0 && m_maxTokenAge < 15) {
        RLOG(WARNING) << "Invalid value for [max_token_age]. Setting it to minimum [15]";
        m_maxTokenAge = 15;
    }
    bool hasTokenAgeLimit = m_maxTokenAge > 0;

    m_tokenAge = jsonDoc.getUInt("token_age", std::min(3600U, m_maxTokenAge));
    if (m_tokenAge == 0 && hasTokenAgeLimit) {
        errorStream << "Cannot set token age [token_age] to 'forever' as [max_token_age] is " << m_maxTokenAge;
    } else if (m_tokenAge > m_maxTokenAge && hasTokenAgeLimit) {
        errorStream << "Cannot set token age [token_age] greater than [max_token_age] which is " << m_maxTokenAge;
    } else if (m_tokenAge != 0 && m_tokenAge < 15) {
        RLOG(WARNING) << "Invalid value for [token_age]. Setting it to minimum [15]";
        m_tokenAge = 15;
    }

    unsigned int defaultClientIntegrityTaskInterval = std::max(300U, std::min(m_clientAge, m_nonAcknowledgedClientAge));
    m_clientIntegrityTaskInterval = jsonDoc.getUInt("client_integrity_task_interval", defaultClientIntegrityTaskInterval);
    if (m_clientIntegrityTaskInterval == 0 || m_clientIntegrityTaskInterval < std::min(m_clientAge, m_nonAcknowledgedClientAge)) {
        RLOG(WARNING) << "Invalid value for [client_integrity_task_interval (" << m_clientIntegrityTaskInterval << ")]. "
                      << "Choose anything greater than or equal to " << std::min(m_clientAge, m_nonAcknowledgedClientAge) << " but not zero. Setting it to lower ["
                      << std::min(m_clientAge, m_nonAcknowledgedClientAge) << "]";
        m_clientIntegrityTaskInterval = defaultClientIntegrityTaskInterval;
    }
    m_dispatchDelay = jsonDoc.getUInt("dispatch_delay", 1);
    if (m_dispatchDelay > 500) {
        RLOG(WARNING) << "Invalid value for [dispatch_delay]. Setting it to default [1ms]";
        m_dispatchDelay = 1;
    }
    m_maxItemsInBulk = jsonDoc.getUInt("max_items_in_bulk", 5);
    if (m_maxItemsInBulk <= 1 || m_maxItemsInBulk >= 100) {
        errorStream << "  Invalid value for [max_items_in_bulk]. Please choose between 2-100" << std::endl;
    }

    // We load known loggers before known clients because
    // known clients may have "loggers" array
    // that will be cross-checked with loggers list

    if (jsonDoc.hasKey("known_loggers")) {
        loadKnownLoggers(root["known_loggers"], errorStream, false);
    }
    auto queryEndpoint = [&](const std::string& endpoint,
            const std::string& keyName,
            const std::function<void(const JsonItem&)>& cb) {

        RVLOG(RV_INFO) << "Querying [" << endpoint << "]...";
        std::string contents;

        try {
            contents = HttpClient::fetchUrlContents(endpoint);
            Utils::trim(contents);
            if (!contents.empty()) {
                JsonDocument jsonRemote(std::move(contents));
                if (jsonRemote.isValid()) {
                    if (jsonRemote.hasKey(keyName)) {
                        JsonItem jsonRemoteRoot = jsonRemote.root();
                        cb(jsonRemoteRoot[keyName]);
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

    if (jsonDoc.hasKey("known_loggers_endpoint")) {
        m_knownLoggersEndpoint = jsonDoc.getString("known_loggers_endpoint");
        if (!m_knownLoggersEndpoint.empty()) {
            queryEndpoint(m_knownLoggersEndpoint, "known_loggers", [&](const JsonItem& json) {
                loadKnownLoggers(json, errorStream, true);
            });
        }
    }

    if (jsonDoc.hasKey("known_clients")) {
        loadKnownClients(root["known_clients"], errorStream, false);
    }

    if (jsonDoc.hasKey("known_clients_endpoint")) {
        m_knownClientsEndpoint = jsonDoc.getString("known_clients_endpoint");
        if (!m_knownClientsEndpoint.empty()) {
            queryEndpoint(m_knownClientsEndpoint, "known_clients", [&](const JsonItem& json) {
                loadKnownClients(json, errorStream, true);
            });
        }
    }

    if (jsonDoc.hasKey("loggers_blacklist")) {
        loadLoggersBlacklist(root["loggers_blacklist"], errorStream);
    }

    #ifdef RESIDUE_HAS_EXTENSIONS
    if (jsonDoc.hasKey("extensions")) {
        jsonDoc jExtensions(root["extensions"]);
        if (jExtensions.hasKey("log_extensions")) {
            JsonValue jExtensionsRoot = jExtensions.root();
            loadExtensions<LogExtension>(jExtensionsRoot["log_extensions"], errorStream, &m_logExtensions);
        }
    }

    #endif
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

#ifdef RESIDUE_USE_GASON

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

        if (j.get<bool>("allow_plain_log_request", hasFlag(Configuration::Flag::ALLOW_PLAIN_LOG_REQUEST))) {
            addLoggerFlag(loggerId, Configuration::Flag::ALLOW_PLAIN_LOG_REQUEST);
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

        JsonDoc::Value accessCodesBlacklist = j.get<JsonDoc::Value>("access_codes_blacklist", JsonDoc::Value());
        if (accessCodesBlacklist.isArray()) {
            JsonDoc jAccessCode(accessCodesBlacklist);

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
                const auto& it = m_accessCodesBlacklist.find(loggerId);
                if (it == m_accessCodesBlacklist.end()) {
                    std::unordered_set<std::string> singleCodeSet = {code};
                    m_accessCodesBlacklist[loggerId] = singleCodeSet;
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

                        // folowing is only for saving purposes
                        m_knownClientUserMap.insert(std::make_pair(clientId, loggerUser));
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

#else

void Configuration::loadKnownLoggers(const JsonItem& json, std::stringstream& errorStream, bool viaUrl)
{
    for (const auto& logger : json) {
        JsonDocument jsonDoc(logger);
        std::string loggerId = jsonDoc.getString("logger_id");
        if (loggerId.empty()) {
            errorStream << "  Logger ID not provided in known_loggers" << std::endl;
            continue;
        }
        if (m_configurations.find(loggerId) != m_configurations.end()) {
            errorStream << "  Duplicate logger in known_loggers [" << loggerId << "]" << std::endl;
            continue;
        }
        std::string easyloggingConfigFile = jsonDoc.getString("configuration_file");
        if (!easyloggingConfigFile.empty()) {
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
        } else {
            errorStream << "  Please specify Easylogging++ configuration for known logger [" << loggerId << "]" << std::endl;
            continue;
        }

        std::string loggerUser = jsonDoc.getString("user");
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

        if (jsonDoc.getBool("allow_plain_log_request", hasFlag(Configuration::Flag::ALLOW_PLAIN_LOG_REQUEST))) {
            addLoggerFlag(loggerId, Configuration::Flag::ALLOW_PLAIN_LOG_REQUEST);
        }

        std::string rotationFreq = jsonDoc.getString("rotation_freq");
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

        std::string archivedLogFilename = jsonDoc.getString("archived_log_filename");
        if (!archivedLogFilename.empty()) {
            m_archivedLogsFilenames.insert(std::make_pair(loggerId, "%logger-" + archivedLogFilename));
        }

        std::string archivedLogCompressedFilename = jsonDoc.getString("archived_log_compressed_filename");
        if (!archivedLogCompressedFilename.empty()) {
            if (archivedLogCompressedFilename.find("/") != std::string::npos || archivedLogCompressedFilename.find("\\") != std::string::npos) {
                errorStream << "  archived_log_compressed_filename contains illegal character (path character). It should be pure filename format." << std::endl;
            } else {
                m_archivedLogsCompressedFilenames.insert(std::make_pair(loggerId, archivedLogCompressedFilename));
            }
        }

        std::string archivedLogDirectory = jsonDoc.getString("archived_log_directory");
        if (!archivedLogDirectory.empty()) {
            m_archivedLogsDirectories.insert(std::make_pair(loggerId, archivedLogDirectory));
        }

        // Access codes are used to generate the tokens
        if (jsonDoc.hasKey("access_codes")) {
            for (const auto& accessCode : logger["access_codes"]) {
                std::string accessCodeStr = accessCode["code"];
                if (accessCodeStr.empty() || accessCodeStr == DEFAULT_ACCESS_CODE) {
                    continue;
                }
                unsigned int age = tokenAge();
                if (accessCode.count("token_age") > 0) {
                    age = accessCode["token_age"];
                    if (m_maxTokenAge > 0 && age > m_maxTokenAge) {
                        errorStream << "Cannot set token age for logger [" << loggerId << "], access code ["
                                    << accessCodeStr << "] greater than [max_token_age] which is " << m_maxTokenAge;
                    } else if (m_maxTokenAge > 0 && age == 0) {
                        errorStream << "Cannot set token age for logger [" << loggerId << "], access code ["
                                    << accessCodeStr << "] to 'forever' [max_token_age] is " << m_maxTokenAge;
                    }
                }
                const auto& it = m_accessCodes.find(loggerId);
                if (it == m_accessCodes.end()) {
                    std::unordered_set<AccessCode> singleCodeSet = { AccessCode(accessCodeStr, age) };
                    m_accessCodes[loggerId] = singleCodeSet;
                } else {
                    it->second.insert(AccessCode(accessCodeStr, age));
                }
            }
        }

        if (jsonDoc.hasKey("access_codes_blacklist")) {
            for (const auto& accessCode : logger["access_codes_blacklist"]) {
                std::string accessCodeStr = accessCode;
                if (accessCodeStr.empty()) {
                    continue;
                }
                if (isValidAccessCode(loggerId, accessCodeStr)) {
                    errorStream << "  Access code [" << accessCodeStr << "] exist in both allowed and blacklist lists" << std::endl;
                    continue;
                }
                const auto& it = m_accessCodesBlacklist.find(loggerId);
                if (it == m_accessCodesBlacklist.end()) {
                    std::unordered_set<std::string> singleCodeSet = {accessCodeStr};
                    m_accessCodesBlacklist[loggerId] = singleCodeSet;
                } else {
                    if (std::find(it->second.begin(), it->second.end(), accessCodeStr) == it->second.end()) {
                        it->second.insert(accessCodeStr);
                    }
                }
            }
        }
    }
}

void Configuration::loadKnownClients(const JsonItem& json, std::stringstream& errorStream, bool viaUrl)
{
    for (const auto& knownClientPair : json) {
        JsonDocument knownClientjsonDoc(knownClientPair);
        std::string clientId = knownClientjsonDoc.getString("client_id");
        if (clientId.empty()) {
            errorStream << "  Client ID not provided in known_clients" << std::endl;
            continue;
        }
        if (!Utils::isAlphaNumeric(clientId, "-_@#")) {
            errorStream << "  Invalid character in client ID, should be alpha-numeric (can also include these characters excluding square brackets: [_@-#])" << std::endl;
            continue;
        }
        std::string publicKey = knownClientjsonDoc.getString("public_key");
        if (publicKey.empty()) {
            errorStream << "  RSA public key not provided in known_clients for [" << clientId << "]" << std::endl;
            continue;
        }
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
        std::string publicKeyContents = std::string(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());

        m_knownClientsKeys.insert(std::make_pair(clientId, std::make_pair(publicKey, publicKeyContents)));

        if (viaUrl) {
            m_remoteKnownClients.insert(clientId);
        }

        if (knownClientjsonDoc.hasKey("key_size")) {
            unsigned int keySize = knownClientjsonDoc.getUInt("key_size", 0);
            if (keySize != 128 && keySize != 192 && keySize != 256) {
                errorStream << "  Invalid key size [" << keySize << "] for client [" << clientId << "]. Please choose 128, 192 or 256-bit" << std::endl;
            } else {
                m_keySizes.insert(std::make_pair(clientId, keySize));
            }
        }

        if (knownClientjsonDoc.hasKey("loggers")) {
            std::unordered_set<std::string> loggerIds;
            for (const std::string& loggerId : knownClientPair["loggers"]) {
                if (!isKnownLogger(loggerId)) {
                    errorStream << "  Logger [" << loggerId << "] for client [" << clientId << "] is unknown" << std::endl;
                    continue;
                }
                loggerIds.insert(loggerId);
            }
            m_knownClientsLoggers.insert(std::make_pair(clientId, loggerIds));

            if (knownClientjsonDoc.hasKey("default_logger")) {
                std::string defaultLogger = knownClientjsonDoc.getString("default_logger");
                if (loggerIds.find(defaultLogger) != loggerIds.end()) {
                    m_knownClientDefaultLogger.insert(std::make_pair(clientId, defaultLogger));
                } else {
                    errorStream << "  Default logger ["  << defaultLogger << "] for client [" << clientId << "] is not part of [loggers] array";
                }
            }

            // add
            // we have user inside loggers because we need to check for different user from loggerIds
            if (knownClientjsonDoc.hasKey("user")) {
                std::string loggerUser = knownClientjsonDoc.getString("user");
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

                            // folowing is only for saving purposes
                            m_knownClientUserMap.insert(std::make_pair(clientId, loggerUser));
                        } else {
                            // for same user ignore, for different user this is a config warning
                            std::string existingAssignedUser = m_knownLoggerUserMap.at(loggerId);
                            if (existingAssignedUser != loggerUser) {
                                RLOG(WARNING) << "  User for logger [" << loggerId << "] has explicit user [" << existingAssignedUser << "]";
                            }
                        }
                    }
                }
            }
        } else {
            // no loggers array
            if (knownClientjsonDoc.hasKey("default_logger")) {
                std::string defaultLogger = knownClientjsonDoc.getString("default_logger");
                errorStream << "  Default logger ["  << defaultLogger << "] for client [" << clientId << "] is not part of [loggers] array";
            }
        }
    }
}

void Configuration::loadLoggersBlacklist(const JsonItem& json, std::stringstream& errorStream)
{
    for (const auto& loggerId : json) {
        std::string loggerIdStr = loggerId;
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

#endif

bool Configuration::save(const std::string& outputFile)
{
    std::lock_guard<std::mutex> lock(m_mutex);
#ifdef RESIDUE_USE_GASON____
    const std::size_t capacity = 1024; // todo: change this
    char source[capacity];

    JsonBuilder j(source, capacity);

    std::string jDump = source;
#else
    JsonItem j;
    j["admin_port"] = adminPort();
    j["connect_port"] = connectPort();
    j["token_port"] = tokenPort();
    j["logging_port"] = loggingPort();
    j["server_key"] = serverKey();
    j["server_rsa_private_key"] = m_serverRSAPrivateKeyFile;
    j["server_rsa_public_key"] = m_serverRSAPublicKeyFile;
    if (!serverRSASecret().empty()) {
        j["server_rsa_secret"] = serverRSASecret();
    }
    j["default_key_size"] = defaultKeySize();
    j["file_mode"] = fileMode();
    j["accept_input"] = hasFlag(Configuration::Flag::ACCEPT_INPUT);
    j["allow_default_access_code"] = hasFlag(Configuration::Flag::ALLOW_DEFAULT_ACCESS_CODE);
    j["allow_plain_connection"] = hasFlag(Configuration::Flag::ALLOW_PLAIN_CONNECTION);
    j["allow_unknown_loggers"] = hasFlag(Configuration::Flag::ALLOW_UNKNOWN_LOGGERS);
    j["allow_unknown_clients"] = hasFlag(Configuration::Flag::ALLOW_UNKNOWN_CLIENTS);
    j["requires_token"] = hasFlag(Configuration::Flag::REQUIRES_TOKEN);
    j["allow_plain_log_request"] = hasFlag(Configuration::Flag::ALLOW_PLAIN_LOG_REQUEST);
    j["immediate_flush"] = hasFlag(Configuration::Flag::IMMEDIATE_FLUSH);
    j["requires_timestamp"] = hasFlag(Configuration::Flag::REQUIRES_TIMESTAMP);
    j["compression"] = hasFlag(Configuration::Flag::COMPRESSION);
    j["allow_bulk_log_request"] = hasFlag(Configuration::Flag::ALLOW_BULK_LOG_REQUEST);
    j["max_items_in_bulk"] = maxItemsInBulk();
    j["token_age"] = tokenAge();
    j["max_token_age"] = maxTokenAge();
    j["timestamp_validity"] = timestampValidity();
    j["client_age"] = clientAge();
    j["non_acknowledged_client_age"] = nonAcknowledgedClientAge();
    j["client_integrity_task_interval"] = clientIntegrityTaskInterval();
    j["dispatch_delay"] = dispatchDelay();
    j["archived_log_directory"] = m_archivedLogDirectory;
    j["archived_log_filename"] = m_archivedLogFilename;
    j["archived_log_compressed_filename"] = m_archivedLogCompressedFilename;

    if (!m_logExtensions.empty()) {
        std::vector<std::string> ext;
        for (auto& e : m_logExtensions) {
            ext.push_back(e->module());
        }
        j["extensions"]["log_extensions"] = ext;
    }

    std::vector<JsonItem> jKnownClients;
    for (auto c : m_knownClientsKeys) {
        if (m_remoteKnownClients.find(c.first) != m_remoteKnownClients.end()) {
            // do not save known clients fetched by URL
            continue;
        }
        JsonItem jKnownClient;
        jKnownClient["client_id"] = c.first;
        jKnownClient["public_key"] = c.second.first; // .first = filename | .second = file contents
        if (m_keySizes.find(c.first) != m_keySizes.end()) {
            jKnownClient["key_size"] = m_keySizes.at(c.first);
        }
        if (m_knownClientsLoggers.find(c.first) != m_knownClientsLoggers.end()) {
            jKnownClient["loggers"] = m_knownClientsLoggers.at(c.first);
        }

        if (m_knownClientUserMap.find(c.first) != m_knownClientUserMap.end()) {
            jKnownClient["user"] = m_knownClientUserMap.at(c.first);
        }
        if (m_knownClientDefaultLogger.find(c.first) != m_knownClientDefaultLogger.end()) {
            jKnownClient["default_logger"] = m_knownClientDefaultLogger.at(c.first);
        }
        jKnownClients.push_back(jKnownClient);
    }
    j["known_clients"] = jKnownClients;
    if (!m_knownClientsEndpoint.empty()) {
        j["known_clients_endpoint"] = m_knownClientsEndpoint;
    }
    j["loggers_blacklist"] = m_blacklist;
    std::vector<JsonItem> jKnownLoggers;
    for (auto c : m_configurations) {
        std::string loggerId = c.first;
        if (m_remoteKnownLoggers.find(loggerId) != m_remoteKnownLoggers.end()) {
            // do not save known loggers fetched by URL
            continue;
        }
        JsonItem jKnownLogger;
        jKnownLogger["logger_id"] = loggerId;
        jKnownLogger["configuration_file"] = c.second;
        if (hasFlag(Configuration::Flag::ALLOW_PLAIN_LOG_REQUEST)) {
            if (!hasLoggerFlag(loggerId, Configuration::Flag::ALLOW_PLAIN_LOG_REQUEST)) {
                jKnownLogger["allow_plain_log_request"] = false;
            }
        } else {
            // no global
            if (hasLoggerFlag(loggerId, Configuration::Flag::ALLOW_PLAIN_LOG_REQUEST)) {
                // but local
                jKnownLogger["allow_plain_log_request"] = true;
            }
        }

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
            jKnownLogger["rotation_freq"] = frequencyStr;
        }

        if (m_archivedLogsFilenames.find(loggerId) != m_archivedLogsFilenames.end()) {
            jKnownLogger["archived_log_filename"] = m_archivedLogsFilenames.at(loggerId).substr(std::string("%logger-").size());
        }

        if (m_accessCodesBlacklist.find(loggerId) != m_accessCodesBlacklist.end() && !m_accessCodesBlacklist.at(loggerId).empty()) {
            jKnownLogger["access_codes_blacklist"] = m_accessCodesBlacklist.at(loggerId);
        }

        if (m_archivedLogsCompressedFilenames.find(loggerId) != m_archivedLogsCompressedFilenames.end()) {
            jKnownLogger["archived_log_compressed_filename"] = m_archivedLogsCompressedFilenames.at(loggerId);
        }

        if (m_archivedLogsDirectories.find(loggerId) != m_archivedLogsDirectories.end()) {
            jKnownLogger["archived_log_directory"] = m_archivedLogsDirectories.at(loggerId);
        }

        if (m_knownLoggerUserMap.find(loggerId) != m_knownLoggerUserMap.end()) {
            jKnownLogger["user"] = m_knownLoggerUserMap.at(loggerId);
        }

        if (m_accessCodes.find(loggerId) != m_accessCodes.end()) {
            std::vector<JsonItem> jAccessCodes;
            for (AccessCode accessCode : m_accessCodes.at(loggerId)) {
                JsonItem jAccessCode;
                jAccessCode["code"] = accessCode.data();
                jAccessCode["token_age"] = accessCode.age();
                jAccessCodes.push_back(jAccessCode);
            }
            jKnownLogger["access_codes"] = jAccessCodes;
        }

        jKnownLoggers.push_back(jKnownLogger);
    }
    j["known_loggers"] = jKnownLoggers;
    if (!m_knownLoggersEndpoint.empty()) {
        j["known_loggers_endpoint"] = m_knownLoggersEndpoint;
    }
    std::string jDump = j.dump(4);
#endif

    std::fstream file;
    file.open(outputFile, std::ofstream::out);
    if (!file) {
        RLOG(ERROR) << "Unable to open file for writing [" << outputFile << "]";
        return false;
    }
    file << jDump;
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
    const auto& blacklistIter = m_accessCodesBlacklist.find(loggerId);
    if (blacklistIter != m_accessCodesBlacklist.end()) {
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

std::string Configuration::getConfigurationFile(const std::string& loggerId, const UserLogBuilder* userLogBuilder) const
{
    return getConfigurationFile(loggerId, userLogBuilder != nullptr ? userLogBuilder->request() : nullptr);
}

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
