//
//  configuration-test.h
//  Residue
//
//  Copyright 2017-present Amrayn Web Services
//  https://amrayn.com
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

#include "test.h"

#include <cstdio>
#include <cstdlib>

#include <fstream>
#include <memory>

#include "ripe/Ripe.h"

#include "connect/connection-request.h"
#include "core/configuration.h"
#include "core/registry.h"
#include "core/residue-exception.h"
#include "logging/client-queue-processor.h"
#include "logging/log-request-handler.h"
#include "logging/user-log-builder.h"
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
        fs << std::string(R"(* GLOBAL:
                              FORMAT                  =   "%datetime [%logger] [%app] %level %msg"
                              FILENAME                =   "/tmp/logs/default.log"
                              ENABLED                 =   true
                              TO_FILE                 =   true
                              SUBSECOND_PRECISION      =   3
                              PERFORMANCE_TRACKING    =   false
                          * VERBOSE:
                              FORMAT                  =   "%datetime [%logger] %app %level-%vlevel %msg")");
        fs.flush();
        fs.close();
        fs.open(kLoggerConfResidue, std::fstream::out);
        fs << std::string(R"(* GLOBAL:
                              FORMAT                  =   "%datetime [%logger] [%app] %level %msg"
                              FILENAME                =   "/tmp/logs/residue.log"
                              ENABLED                 =   true
                              TO_FILE                 =   true
                              SUBSECOND_PRECISION      =   3
                              PERFORMANCE_TRACKING    =   false
                          * VERBOSE:
                              FORMAT                  =   "%datetime [%logger] %app %level-%vlevel %msg")");

        fs.flush();
        fs.close();
        fs.open(kLoggerConfMuflihun, std::fstream::out);
        fs << std::string(R"(* GLOBAL:
                              FORMAT                  =   "%datetime [%logger] [%app] %level %msg"
                              FILENAME                =   "/tmp/logs/muflihun.log"
                              ENABLED                 =   true
                              TO_FILE                 =   true
                              SUBSECOND_PRECISION      =   3
                              PERFORMANCE_TRACKING    =   false
                          * VERBOSE:
                              FORMAT                  =   "%datetime [%logger] %app %level-%vlevel %msg")");
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
                              "logging_port": 87791,
                              "server_key": "048CB7050312DB329788CE1533C294A1F248F8A1BD6F611D7516803EDE271C65",
                              "allow_unmanaged_loggers": true,
                              "allow_unmanaged_clients": false,
                              "enable_cli": false,
                              "allow_insecure_connection": true,
                              "immediate_flush": true,
                              "allow_bulk_log_request": true,
                              "client_integrity_task_interval": 500,
                              "client_age": 2147483648,
                              "compression":false,
                              "non_acknowledged_client_age": 400,
                              "dispatch_delay": 1,
                              "archived_log_directory": "%original",
                              "archived_log_filename": "mylogs-%hour-00-%wday-%level.log",
                              "archived_log_compressed_filename": "mylogs-%hour-00-%wday.tar.gz",
                              "managed_clients": [
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
                              "managed_loggers": [
                                  {
                                      "logger_id": "residue",
                                      "configuration_file": "residue-logger.conf",
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
                                      "rotation_freq": "hourly",
                                      "archived_log_filename": "mylogs-%hour-00-%wday-%level.log",
                                      "archived_log_directory": "/tmp/logs-backup/custom-location-for-%logger"
                                  }
                              ],
                              "managed_loggers_endpoint": "http://localhost:3000/managed-loggers",
                              "managed_clients_endpoint": "http://localhost:3000/managed-clients",
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
    ASSERT_EQ(conf->loggingPort(), 87791);
    ASSERT_EQ(conf->serverKey(), "048CB7050312DB329788CE1533C294A1F248F8A1BD6F611D7516803EDE271C65");
    ASSERT_EQ(conf->clientAge(), 2147483648);
    ASSERT_EQ(conf->nonAcknowledgedClientAge(), 400);
    ASSERT_EQ(conf->clientIntegrityTaskInterval(), 500);
    ASSERT_FALSE(conf->hasFlag(Configuration::Flag::ENABLE_CLI));
    ASSERT_TRUE(conf->hasFlag(Configuration::Flag::ALLOW_UNMANAGED_LOGGERS));
    ASSERT_FALSE(conf->hasFlag(Configuration::Flag::ALLOW_UNMANAGED_CLIENTS));
    ASSERT_FALSE(conf->hasFlag(Configuration::Flag::COMPRESSION));
    ASSERT_TRUE(conf->hasFlag(Configuration::Flag::ALLOW_INSECURE_CONNECTION));
    ASSERT_FALSE(conf->isManagedLoggerForClient("missing-client", "muflihun"));
    ASSERT_FALSE(conf->isManagedLoggerForClient("client-for-test", "missing-logger"));
    ASSERT_TRUE(conf->isManagedLoggerForClient("client-for-test", "muflihun"));
    ASSERT_FALSE(conf->isManagedLoggerForClient("client-for-test", "default"));
    ASSERT_EQ(conf->managedClientsKeys().size(), 2);
    ASSERT_EQ(conf->keySize("client-for-test"), 128);
    ASSERT_EQ(conf->keySize("client-for-test2"), 256);
    ASSERT_EQ(conf->getConfigurationFile("muflihun"), "muflihun-logger.conf");

    ASSERT_EQ(conf->managedLoggersEndpoint(), "http://localhost:3000/managed-loggers");
    ASSERT_EQ(conf->managedClientsEndpoint(), "http://localhost:3000/managed-clients");
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
    ASSERT_EQ(conf2->loggingPort(), conf->loggingPort());
    ASSERT_EQ(conf2->serverKey(), conf->serverKey());
    ASSERT_EQ(conf2->clientAge(), conf->clientAge());
    ASSERT_EQ(conf2->managedClientsKeys().size(), conf->managedClientsKeys().size());
    ASSERT_EQ(conf2->nonAcknowledgedClientAge(), conf->nonAcknowledgedClientAge());
    ASSERT_EQ(conf2->clientIntegrityTaskInterval(), conf->clientIntegrityTaskInterval());
    ASSERT_EQ(conf2->hasFlag(Configuration::Flag::ENABLE_CLI), conf->hasFlag(Configuration::Flag::ENABLE_CLI));
    ASSERT_EQ(conf2->hasFlag(Configuration::Flag::ALLOW_UNMANAGED_LOGGERS), conf->hasFlag(Configuration::Flag::ALLOW_UNMANAGED_LOGGERS));
    ASSERT_EQ(conf2->hasFlag(Configuration::Flag::ALLOW_UNMANAGED_CLIENTS), conf->hasFlag(Configuration::Flag::ALLOW_UNMANAGED_CLIENTS));
    ASSERT_FALSE(conf2->isManagedLoggerForClient("missing-client", "muflihun"));
    ASSERT_FALSE(conf2->isManagedLoggerForClient("client-for-test", "missing-logger"));
    ASSERT_TRUE(conf2->isManagedLoggerForClient("client-for-test", "muflihun"));
    ASSERT_FALSE(conf2->isManagedLoggerForClient("client-for-test", "default"));
    ASSERT_EQ(conf2->managedClientsKeys().size(), 2);
    ASSERT_EQ(conf2->keySize("client-for-test"), 128);
    ASSERT_EQ(conf2->keySize("client-for-test2"), 256);
    ASSERT_EQ(conf2->logExtensions().size(), conf->logExtensions().size());
    ASSERT_EQ(conf2->preArchiveExtensions().size(), conf->preArchiveExtensions().size());
    ASSERT_EQ(conf2->postArchiveExtensions().size(), conf->postArchiveExtensions().size());
    ASSERT_EQ(conf2->getConfigurationFile("muflihun"), "muflihun-logger.conf");
    ASSERT_EQ(conf2->getConfigurationFile("unmanagedlogger"), "");
}


TEST_F(ConfigurationTest, ManagedLoggersRequestAllowed)
{

    // Setup basic request
    Registry registry(conf.get());
    ClientIntegrityTask task(&registry, 300);
    registry.setClientIntegrityTask(&task);
    ClientQueueProcessor logProcessor(&registry, "");
    logProcessor.start(); // start to handle ~logProcessor
    conf->removeFlag(Configuration::REQUIRES_TIMESTAMP);
    std::string connectionRequestStr(R"({
                                            "client_id":"blah",
                                            "type":1,
                                            "key_size":256
                                        })");
    ConnectionRequest connectionReq(registry.configuration());
    connectionReq.setDateReceived(1000);
    connectionReq.deserialize(std::move(connectionRequestStr));
    Client client(&connectionReq);
    client.setIsManaged(true);
    ASSERT_EQ(client.keySize(), 32);
    registry.addClient(client);

    Client unmanagedClient(&connectionReq);
    unmanagedClient.setIsManaged(false);
    ASSERT_EQ(unmanagedClient.keySize(), 32);
    registry.addClient(unmanagedClient);

    auto createLogRequest = [](const std::string& loggerId) -> std::string {
        return std::string(R"(
                           {"client_id":"blah",
                               "datetime" : 123,
                               "logger": ")" + loggerId + R"(",
                               "_t": ")") + std::to_string(Utils::now()) + std::string(R"(",
                               "msg": "Efficient real-time centralized logging server",
                               "file": "index.html",
                               "line": 857,
                               "app": "Muflihun.com",
                               "level": 4
                            })");
    };

    auto runTests = [&](const TestData<std::string, Client*, bool>& testCases) {
        for (auto& t : testCases) {
            std::string r1 = createLogRequest(t.get<0>());
            LogRequest logRequest(registry.configuration());
            logRequest.setDateReceived(Utils::now());
            logRequest.deserialize(std::move(r1));
            logRequest.setClient(t.get<1>());
            ASSERT_EQ(logProcessor.isRequestAllowed(&logRequest), t.get<2>())
                    << "Logger: " << t.get<0>() << " Client: " << t.get<1>()->id();
        }
    };

    TestData<std::string, Client*, bool> testCases = {
        { "muflihun", &client, true },
        { "default", &client, true },
        { "residue", &client, false }, // residue is not allowed in any case
        { "test", &client, true },
        { "muflihun", &unmanagedClient, false },
        { "default", &unmanagedClient, true },
        { "residue", &unmanagedClient, false },
        { "test", &unmanagedClient, true }
    };
    runTests(testCases);

    conf->removeFlag(Configuration::ALLOW_UNMANAGED_LOGGERS);

    TestData<std::string, Client*, bool> testCases2 = {
        { "muflihun", &client, true },
        { "default", &client, true },
        { "residue", &client, false }, // residue is not allowed in any case
        { "test", &client, false }, // unmanaged
        { "muflihun", &unmanagedClient, false },
        { "default", &unmanagedClient, true },
        { "residue", &unmanagedClient, false },
        { "test", &unmanagedClient, false }
    };
    runTests(testCases2);

    // reset it back
    conf->addFlag(Configuration::REQUIRES_TIMESTAMP);
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
