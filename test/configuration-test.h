//
//  configuration-test.h
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

#ifndef CONFIGURATION_TEST_H
#define CONFIGURATION_TEST_H

#include <cstdio>
#include <fstream>
#include <memory>
#include "ripe/Ripe.h"
#include "test.h"
#include "core/residue-exception.h"
#include "../src/core/configuration.h"
#include "core/registry.h"
#include "logging/user-log-builder.h"
#include "logging/log-request-handler.h"
#include "connect/connection-request.h"
#include "tasks/client-integrity-task.h"

using namespace residue;

static const char* kConfigurationFile = "configuration.json";
static const char* kLoggerConfDefault = "default-logger.conf";
static const char* kLoggerConfDefaultSaved = "saved-default-logger-conf.conf";
static const char* kLoggerConfResidue = "residue-logger.conf";
static const char* kLoggerConfMuflihun = "muflihun-logger.conf";
static const char* kPrivateKeyFile = "private-key-for-test.pem";
static const char* kPublicKeyFile = "public-key-for-test.pem";

class ConfigurationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::fstream fs;
        // Logger confs
        fs.open(kLoggerConfDefault, std::fstream::out);
        fs << std::string(R"(
                          * GLOBAL:
                              FORMAT                  =   "%datetime [%logger] [%app] %level %msg"
                              FILENAME                =   "/tmp/logs/default.log"
                              ENABLED                 =   true
                              TO_FILE                 =   true
                              SUBSECOND_PRECISION      =   3
                              PERFORMANCE_TRACKING    =   false
                          * VERBOSE:
                              FORMAT                  =   "%datetime [%logger] %app %level-%vlevel %msg"
                          ")");
        fs.flush();
        fs.close();
        fs.open(kLoggerConfResidue, std::fstream::out);
        fs << std::string(R"(
                          * GLOBAL:
                              FORMAT                  =   "%datetime [%logger] [%app] %level %msg"
                              FILENAME                =   "/tmp/logs/residue.log"
                              ENABLED                 =   true
                              TO_FILE                 =   true
                              SUBSECOND_PRECISION      =   3
                              PERFORMANCE_TRACKING    =   false
                          * VERBOSE:
                              FORMAT                  =   "%datetime [%logger] %app %level-%vlevel %msg"
                          ")");

        fs.flush();
        fs.close();
        fs.open(kLoggerConfMuflihun, std::fstream::out);
        fs << std::string(R"(
                          * GLOBAL:
                              FORMAT                  =   "%datetime [%logger] [%app] %level %msg"
                              FILENAME                =   "/tmp/logs/muflihun.log"
                              ENABLED                 =   true
                              TO_FILE                 =   true
                              SUBSECOND_PRECISION      =   3
                              PERFORMANCE_TRACKING    =   false
                          * VERBOSE:
                              FORMAT                  =   "%datetime [%logger] %app %level-%vlevel %msg"
                          ")");
        fs.flush();
        fs.close();

        // keys
        LOG(INFO) << "generating keypair";
        Ripe::writeRSAKeyPair(kPublicKeyFile, kPrivateKeyFile);
        LOG(INFO) << "keypair saved";
        // Residue server conf
        LOG(INFO) << "generating conf";
        fs.open (kConfigurationFile, std::fstream::out);
        fs << std::string(R"(
                          {
                              "admin_port": 87761,
                              "connect_port": 87771,
                              "token_port": 87781,
                              "logging_port": 87791,
                              "server_key": "048CB7050312DB329788CE1533C294A1F248F8A1BD6F611D7516803EDE271C65",
                              "allow_default_access_code": true,
                              "allow_unknown_loggers": true,
                              "allow_unknown_clients": false,
                              "enable_cli": false,
                              "requires_token": true,
                              "allow_plain_log_request": true,
                              "immediate_flush": true,
                              "allow_bulk_log_request": true,
                              "client_integrity_task_interval": 500,
                              "client_age": 0,
                              "token_age": 25,
                              "max_token_age": 60,
                              "non_acknowledged_client_age": 3600,
                              "dispatch_delay": 1,
                              "archived_log_directory": "%original",
                              "archived_log_filename": "mylogs-%hour-00-%wday-%level.log",
                              "archived_log_compressed_filename": "mylogs-%hour-00-%wday.tar.gz",
                              "known_clients": [
                                  {
                                      "client_id": "client-for-test",
                                      "public_key": "public-key-for-test.pem",
                                      "key_size": 128,
                                      "loggers": ["muflihun"],
                                      "default_logger": "muflihun"
                                  },
                                  {
                                      "client_id": "client-for-test2",
                                      "public_key": "public-key-for-test.pem",
                                      "loggers": ["muflihun"]
                                  }
                              ],
                              "known_loggers": [
                                  {
                                      "logger_id": "residue",
                                      "configuration_file": "residue-logger.conf",
                                      "allow_plain_log_request": false
                                  },
                                  {
                                      "logger_id": "default",
                                      "configuration_file": "default-logger.conf",
                                      "rotation_freq": "daily",
                                      "archived_log_filename": "mylogs-%hour-00-%wday-%level.log"
                                  },
                                  {
                                      "logger_id": "muflihun",
                                      "configuration_file": "muflihun-logger.conf",
                                      "max_file_size": 102400000,
                                      "rotation_freq": "hourly",
                                      "archived_log_filename": "mylogs-%hour-00-%wday-%level.log",
                                      "archived_log_directory": "/tmp/logs-backup/custom-location-for-%logger",
                                      "access_code_blacklist": [
                                          "ii3faf",
                                          "ii3fa2"
                                      ],
                                      "access_codes": [
                                          {
                                              "code": "a2dcb",
                                              "token_age": 60
                                          },
                                          {
                                              "code": "eif89",
                                              "token_age": 20
                                          },
                                          {
                                              "code": "eif82"
                                          }
                                      ]
                                  }
                              ],
                              "extensions": {
                                  "log_extensions": ["basic"]
                              },
                              "loggers_blacklist": [
                                  "bracket",
                                  "truli"
                              ]
                          }

                          )");
        fs.flush();
        fs.close();
        LOG(INFO) << "conf saved";
        conf = std::unique_ptr<Configuration>(new Configuration(kConfigurationFile));
        if (!conf->isValid()) {
            std::cout << "ERRORS: " << conf->errors() << std::endl;
        }
    }

    void TearDown() override
    {
        remove(kLoggerConfDefault);
        remove(kLoggerConfResidue);
        remove(kLoggerConfMuflihun);
        remove(kConfigurationFile);
    }

    std::unique_ptr<Configuration> conf;
};

TEST_F(ConfigurationTest, CheckValues)
{
    ASSERT_EQ(conf->adminPort(), 87761);
    ASSERT_EQ(conf->connectPort(), 87771);
    ASSERT_EQ(conf->tokenPort(), 87781);
    ASSERT_EQ(conf->loggingPort(), 87791);
    ASSERT_EQ(conf->serverKey(), "048CB7050312DB329788CE1533C294A1F248F8A1BD6F611D7516803EDE271C65");
    ASSERT_EQ(conf->clientAge(), 0);
    ASSERT_EQ(conf->tokenAge(), 25);
    ASSERT_EQ(conf->maxTokenAge(), 60);
    ASSERT_EQ(conf->nonAcknowledgedClientAge(), 3600);
    ASSERT_EQ(conf->clientIntegrityTaskInterval(), 500);
    ASSERT_FALSE(conf->hasFlag(Configuration::Flag::ENABLE_CLI));
    ASSERT_TRUE(conf->hasFlag(Configuration::Flag::ALLOW_UNKNOWN_LOGGERS));
    ASSERT_FALSE(conf->hasFlag(Configuration::Flag::ALLOW_UNKNOWN_CLIENTS));
    ASSERT_TRUE(conf->hasFlag(Configuration::Flag::ALLOW_DEFAULT_ACCESS_CODE));
    ASSERT_TRUE(conf->hasFlag(Configuration::Flag::REQUIRES_TOKEN));
    ASSERT_TRUE(conf->hasFlag(Configuration::Flag::ALLOW_PLAIN_LOG_REQUEST));
    ASSERT_FALSE(conf->hasLoggerFlag("residue", Configuration::Flag::ALLOW_PLAIN_LOG_REQUEST));
    ASSERT_TRUE(conf->hasLoggerFlag("muflihun", Configuration::Flag::ALLOW_PLAIN_LOG_REQUEST));
    ASSERT_FALSE(conf->isKnownLoggerForClient("missing-client", "muflihun"));
    ASSERT_FALSE(conf->isKnownLoggerForClient("client-for-test", "missing-logger"));
    ASSERT_TRUE(conf->isKnownLoggerForClient("client-for-test", "muflihun"));
    ASSERT_FALSE(conf->isKnownLoggerForClient("client-for-test", "default"));
    ASSERT_EQ(conf->knownClientsKeys().size(), 2);
    ASSERT_EQ(conf->keySize("client-for-test"), 128);
    ASSERT_EQ(conf->keySize("client-for-test2"), 256);
    ASSERT_EQ(conf->getConfigurationFile("muflihun"), "muflihun-logger.conf");
    ASSERT_TRUE(conf->isValidAccessCode("muflihun", "a2dcb"));
    ASSERT_FALSE(conf->isValidAccessCode("muflihun", "a2dca"));
    ASSERT_FALSE(conf->isValidAccessCode("residue", "a2dcb"));
    ASSERT_FALSE(conf->isValidAccessCode("default", "a2dcb"));
#ifdef RESIDUE_HAS_EXTENSIONS
    ASSERT_EQ(conf->logExtensions().size(), 1);
#else
    ASSERT_EQ(conf->logExtensions().size(), 0);
#endif

    LogRequest r(conf.get());
    r.setClientId("client-for-test");
    ASSERT_EQ(conf->getConfigurationFile("unknownlogger", &r), "muflihun-logger.conf");

    r.setClientId("client-for-test2");
    ASSERT_EQ(conf->getConfigurationFile("unknownlogger", &r), "");

    r.setClientId("unknown-client");
    ASSERT_EQ(conf->getConfigurationFile("unknownlogger", &r), "");
}

TEST_F(ConfigurationTest, Load)
{
    ASSERT_TRUE(conf->isValid());
}

TEST_F(ConfigurationTest, Save)
{
    ASSERT_TRUE(conf->save(kLoggerConfDefaultSaved));
    Configuration loadedSavedConf;
    loadedSavedConf.load(kLoggerConfDefaultSaved);
    ASSERT_TRUE(loadedSavedConf.isValid());

    Configuration* conf2 = &loadedSavedConf;
    ASSERT_EQ(conf2->adminPort(), conf->adminPort());
    ASSERT_EQ(conf2->connectPort(), conf->connectPort());
    ASSERT_EQ(conf2->tokenPort(), conf->tokenPort());
    ASSERT_EQ(conf2->loggingPort(), conf->loggingPort());
    ASSERT_EQ(conf2->serverKey(), conf->serverKey());
    ASSERT_EQ(conf2->clientAge(), conf->clientAge());
    ASSERT_EQ(conf2->tokenAge(), conf->tokenAge());
    ASSERT_EQ(conf2->maxTokenAge(), conf->maxTokenAge());
    ASSERT_EQ(conf2->knownClientsKeys().size(), conf->knownClientsKeys().size());
    ASSERT_EQ(conf2->nonAcknowledgedClientAge(), conf->nonAcknowledgedClientAge());
    ASSERT_EQ(conf2->clientIntegrityTaskInterval(), conf->clientIntegrityTaskInterval());
    ASSERT_EQ(conf2->hasFlag(Configuration::Flag::ENABLE_CLI), conf->hasFlag(Configuration::Flag::ENABLE_CLI));
    ASSERT_EQ(conf2->hasFlag(Configuration::Flag::ALLOW_UNKNOWN_LOGGERS), conf->hasFlag(Configuration::Flag::ALLOW_UNKNOWN_LOGGERS));
    ASSERT_EQ(conf2->hasFlag(Configuration::Flag::ALLOW_UNKNOWN_CLIENTS), conf->hasFlag(Configuration::Flag::ALLOW_UNKNOWN_CLIENTS));
    ASSERT_EQ(conf2->hasFlag(Configuration::Flag::ALLOW_DEFAULT_ACCESS_CODE), conf->hasFlag(Configuration::Flag::ALLOW_DEFAULT_ACCESS_CODE));
    ASSERT_EQ(conf2->hasFlag(Configuration::Flag::REQUIRES_TOKEN), conf->hasFlag(Configuration::Flag::REQUIRES_TOKEN));
    ASSERT_EQ(conf2->hasFlag(Configuration::Flag::ALLOW_PLAIN_LOG_REQUEST), conf->hasFlag(Configuration::Flag::ALLOW_PLAIN_LOG_REQUEST));
    ASSERT_EQ(conf2->hasLoggerFlag("residue", Configuration::Flag::ALLOW_PLAIN_LOG_REQUEST), conf->hasLoggerFlag("residue", Configuration::Flag::ALLOW_PLAIN_LOG_REQUEST));
    ASSERT_EQ(conf2->hasLoggerFlag("muflihun", Configuration::Flag::ALLOW_PLAIN_LOG_REQUEST), conf->hasLoggerFlag("muflihun", Configuration::Flag::ALLOW_PLAIN_LOG_REQUEST));
    ASSERT_FALSE(conf2->isKnownLoggerForClient("missing-client", "muflihun"));
    ASSERT_FALSE(conf2->isKnownLoggerForClient("client-for-test", "missing-logger"));
    ASSERT_TRUE(conf2->isKnownLoggerForClient("client-for-test", "muflihun"));
    ASSERT_FALSE(conf2->isKnownLoggerForClient("client-for-test", "default"));
    ASSERT_EQ(conf2->knownClientsKeys().size(), 2);
    ASSERT_EQ(conf2->keySize("client-for-test"), 128);
    ASSERT_EQ(conf2->keySize("client-for-test2"), 256);
    ASSERT_EQ(conf2->logExtensions().size(), conf->logExtensions().size());

    LogRequest r(conf2);
    r.setClientId("client-for-test");
    ASSERT_EQ(conf2->getConfigurationFile("unknownlogger", &r), "muflihun-logger.conf");

    r.setClientId("client-for-test2");
    ASSERT_EQ(conf2->getConfigurationFile("unknownlogger", &r), "");

    r.setClientId("unknown-client");
    ASSERT_EQ(conf2->getConfigurationFile("unknownlogger", &r), "");
}


TEST_F(ConfigurationTest, KnownLoggersRequestAllowed)
{

    // Setup basic request
    UserLogBuilder logBuilder;
    Registry registry(conf.get());
    ClientIntegrityTask task(&registry, 300);
    registry.setClientIntegrityTask(&task);
    LogRequestHandler logRequestHandler(&registry, &logBuilder);
    logRequestHandler.start(); // start to handle ~LogRequestHandler
    // We remove token check for this test
    conf->removeFlag(Configuration::REQUIRES_TOKEN);
    std::string connectionRequestStr(R"({
                                            "client_id":"blah",
                                            "type":1,
                                            "key_size":256,
                                            "_t": 999
                                        })");
    ConnectionRequest connectionReq(registry.configuration());
    connectionReq.setDateReceived(1000);
    connectionReq.deserialize(std::move(connectionRequestStr));
    Client client(&connectionReq);
    ASSERT_EQ(client.keySize(), 32);
    registry.addClient(client);
    auto createLogRequest = [](const std::string& loggerId) -> std::string {
        return std::string(R"(
                                      {"client_id":"blah",
                                          "token": 123,
                                          "datetime" : 123,
                                          "logger": ")" + loggerId + R"(",
                                          "msg": "Efficient real-time centralized logging server",
                                          "file": "index.html",
                                          "line": 857,
                                          "app": "Muflihun.com",
                                          "level": 4
                                       })");
    };

    auto runTests = [&](const std::map<std::string, bool>& testCases) {
        for (auto& t : testCases) {
            std::string r1 = createLogRequest(t.first);
            LogRequest logRequest(registry.configuration());
            logRequest.setDateReceived(Utils::now());
            logRequest.deserialize(std::move(r1));
            logRequest.setClient(&client);
            ASSERT_EQ(logRequestHandler.isRequestAllowed(&logRequest), t.second);
        }
    };

    std::map<std::string, bool> testCases = {
        { "muflihun", true },
        { "default", true },
        { "residue", false }, // residue is not allowed in any case
        { "test", true }
    };
    runTests(testCases);

    conf->removeFlag(Configuration::ALLOW_UNKNOWN_LOGGERS);

    std::map<std::string, bool> testCases2 = {
        { "muflihun", true },
        { "default", true },
        { "residue", false }, // residue is not allowed in any case
        { "test", false } // unknown
    };
    runTests(testCases2);

    // reset it back
    conf->addFlag(Configuration::REQUIRES_TOKEN);
}

TEST_F(ConfigurationTest, AccessCode)
{
    ASSERT_TRUE(conf->isValidAccessCode("muflihun", "a2dcb"));
    ASSERT_FALSE(conf->isValidAccessCode("default", "a2dcb"));
    // Blacklisted access code
    ASSERT_FALSE(conf->isValidAccessCode("muflihun", "ii3faf"));
    ASSERT_FALSE(conf->isValidAccessCode("default", "ii3faf"));

    ASSERT_EQ(conf->getAccessCodeTokenLife("muflihun", "a2dcb"), 60);
    ASSERT_EQ(conf->getAccessCodeTokenLife("muflihun", "eif89"), 20);
    ASSERT_EQ(conf->getAccessCodeTokenLife("muflihun", "eif82"), 25);
}

TEST_F(ConfigurationTest, BlackListedLogger)
{
    ASSERT_FALSE(conf->isBlacklisted("muflihun"));
    ASSERT_FALSE(conf->isBlacklisted("default"));
    ASSERT_FALSE(conf->isBlacklisted("residue"));
    ASSERT_TRUE(conf->isBlacklisted("truli"));
    ASSERT_TRUE(conf->isBlacklisted("bracket"));
}

TEST_F(ConfigurationTest, LoggerConfiguration)
{
    ASSERT_EQ(conf->getConfigurationFile("default"), kLoggerConfDefault);
    ASSERT_EQ(conf->getConfigurationFile("residue"), kLoggerConfResidue);
    ASSERT_EQ(conf->getConfigurationFile("muflihun"), kLoggerConfMuflihun);
}

#endif // CONFIGURATION_TEST_H
