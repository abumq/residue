//
//  client-queue-processor.cc
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

#include "logging/client-queue-processor.h"

#include "core/configuration.h"
#include "logging/log.h"
#include "logging/log-request.h"
#include "logging/user-message.h"
#include "tasks/client-integrity-task.h"

using namespace residue;

ClientQueueProcessor::ClientQueueProcessor(Registry* registry, const std::string& clientId) :
    RequestHandler("Processor", registry),
    m_clientId(clientId),
    m_enabled(true),
    m_stopped(true)
{
    DRVLOG(RV_DEBUG) << "Initialized processor [LogDispatcher<" << m_clientId << ">] @ " << this;
}

ClientQueueProcessor::~ClientQueueProcessor()
{
    RLOG(WARNING) << "~LogDispatcher<" << m_clientId << ">";
    m_stopped = true;
    m_worker.join();
}

void ClientQueueProcessor::start()
{
    if (m_stopped == true) {
        m_stopped = false;
        m_worker = std::thread([&]() {
            el::Helpers::setThreadName("LogDispatcher<" + m_clientId + ">");
            while (!m_stopped) {
                if (m_enabled) {
                    processRequestQueue();
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
        RLOG(INFO) << "Started client processor [LogDispatcher<" << m_clientId << ">]";
    }
}

void ClientQueueProcessor::processRequestQueue()
{
    bool compressionEnabled = m_registry->configuration()->hasFlag(Configuration::Flag::COMPRESSION);
    bool allowBulkRequests = m_registry->configuration()->hasFlag(Configuration::ALLOW_BULK_LOG_REQUEST);
    auto maxItemsInBulk = m_registry->configuration()->maxItemsInBulk();

 #ifdef RESIDUE_PROFILING
    types::Time m_timeTaken;
    RESIDUE_PROFILE_START(t_process_queue);
    std::size_t totalRequests = 0; // 1 for 1 request so for bulk of 50 this will be 50
 #endif

    // we take snapshot to prevent potential race conditions (even though we have LoggingQueue that is safe)
    const std::size_t total = m_queue.size();

    const types::Time lastClientIntegrityRun = m_registry->clientIntegrityTask() == nullptr
            ? 0L : m_registry->clientIntegrityTask()->lastExecution();

    if (total > 0 && m_registry->clientIntegrityTask() != nullptr) {
        // we pause client integrity task until we clear this queue
        // so we don't clean a (now) dead client that passed initial validation
#ifdef RESIDUE_DEV
        DRVLOG(RV_DEBUG) << "Pausing client integrity task for [" << m_clientId << "]";
#endif
        m_registry->clientIntegrityTask()->pauseClient(m_clientId);
    }

    for (std::size_t i = 0; i < total; ++i) {

        if (m_registry->configuration()->dispatchDelay() > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(m_registry->configuration()->dispatchDelay()));
        }

#ifdef RESIDUE_DEBUG
        DRVLOG(RV_CRAZY) << "-----============= [ BEGIN ] =============-----";
#endif
        LogRequest request(m_registry->configuration());
        RawRequest rawRequest = m_queue.pull();

        // get another reference to shared pointer for session
        std::shared_ptr<Session> session = rawRequest.session;

        RequestHandler::handle(std::move(rawRequest), &request, Request::StatusCode::BAD_REQUEST,
                               false, false, compressionEnabled);

        if ((!request.isValid() && !request.isBulk())
                || request.statusCode() != Request::StatusCode::OK) {
            RVLOG(RV_ERROR) << "Failed: " << request.errorText();
            continue;
        }

#ifdef RESIDUE_DEV
        DRVLOG(RV_DEBUG) << "Is bulk? " << request.isBulk();
#endif
        if (request.isBulk()) {
            if (allowBulkRequests) {
                // Create bulk request items
                unsigned int itemCount = 0U;
                Client* currentClient = request.client();
                bool forceClientValidation = true;
 #ifdef RESIDUE_DEV
                DRVLOG(RV_DEBUG) << "Request client: " << request.client();
 #endif
                for (const auto& js : request.jsonObject()) {
                    if (itemCount == maxItemsInBulk) {
                        RLOG(ERROR) << "Maximum number of bulk requests reached. Ignoring the rest of items in bulk";
                        break;
                    }
                    m_jsonDocForBulk.set(js);
                    std::string requestItemStr(m_jsonDocForBulk.dump());
                    LogRequest requestItem(m_registry->configuration());

                    // we need this for timestamp checking
                    requestItem.setDateReceived(request.dateReceived());

                    requestItem.deserialize(std::move(requestItemStr));
                    if (requestItem.isValid()) {
                        requestItem.setIpAddr(request.ipAddr());
                        requestItem.setSessionId(request.sessionId());
                        requestItem.setClient(request.client());

                        if (processRequest(&requestItem, &currentClient, forceClientValidation, session.get())) {
                            forceClientValidation = false;
                        } else {
                            // force next client validation if last process was unsuccessful
                            forceClientValidation = true;
                        }
                        itemCount++;
 #ifdef RESIDUE_PROFILING
                        totalRequests++;
 #endif
                    } else {
                        RLOG(ERROR) << "Invalid request in bulk.";
                    }
                }
            } else {
                RLOG(ERROR) << "Bulk requests are not allowed";
            }
        } else {

            if (request.client() != nullptr) {
                request.setClientId(request.client()->id());
            }
            processRequest(&request, nullptr, true, session.get());
#ifdef RESIDUE_PROFILING
            totalRequests++;
#endif
        }

#ifdef RESIDUE_DEBUG
        DRVLOG(RV_CRAZY) << "-----============= [ ✓ ] =============-----";
#endif
    }

    if (m_registry->clientIntegrityTask() != nullptr &&
            lastClientIntegrityRun < m_registry->clientIntegrityTask()->lastExecution() &&
            m_queue.backlogEmpty()) {
        RVLOG(RV_DEBUG) << "Starting client integrity task after queue is processed.";
        // trigger client integrity task as it was run while this queue was being processed
        if (!m_registry->clientIntegrityTask()->isExecuting()) {
            if (m_clientId != Configuration::UNKNOWN_CLIENT_ID) {
                // Unknown clients are special case as CLIENT ID is not real ID
                // so we execute whole task at next schedule (provided no other unknown client)
                // add more logs to the queue in which case it will be paused again
            } else {
                m_registry->clientIntegrityTask()->performCleanup(m_clientId);
            }
        }
    }

    if (total > 0 && m_registry->clientIntegrityTask() != nullptr && m_queue.backlogEmpty()) {
#ifdef RESIDUE_DEV
        DRVLOG(RV_DEBUG) << "Resuming client integrity task for [" << m_clientId << "]";
#endif
        m_registry->clientIntegrityTask()->resumeClient(m_clientId);
    }

 #ifdef RESIDUE_PROFILING
    RESIDUE_PROFILE_END(t_process_queue, m_timeTaken);
    float timeTakenInSec = static_cast<float>(m_timeTaken / 1000.0f);
    RLOG_IF(total > 0, DEBUG) << "Took " << timeTakenInSec << "s to process the queue of "
                                   << total << " items (" << totalRequests << " requests). Average: "
                                   << (static_cast<float>(m_timeTaken) / static_cast<float>(total)) << "ms/item ["
                                   << (static_cast<float>(m_timeTaken) / static_cast<float>(totalRequests)) << "ms/request]";
 #endif

    m_queue.switchContext();
}

bool ClientQueueProcessor::processRequest(LogRequest* request, Client** clientRef, bool forceCheck, Session *session)
{
    bool bypassChecks = !forceCheck && clientRef != nullptr && *clientRef != nullptr;
 #ifdef RESIDUE_DEV
    DRVLOG(RV_DEBUG_2) << "Force check: " << forceCheck << ", clientRef: " << clientRef << ", *clientRef: "
                     << (clientRef == nullptr ? "N/A" : *clientRef == nullptr ? "null" : (*clientRef)->id())
                     << ", bypassChecks: " << bypassChecks;
 #endif
    Client* client = clientRef != nullptr && *clientRef != nullptr ? *clientRef : request->client();

    if (client == nullptr) {
        RVLOG(RV_ERROR) << "Invalid request. No client found [" << request->clientId() << "]";
        return false;
    }

    if (!bypassChecks && !client->isAlive(request->dateReceived())) {
        RLOG(ERROR) << "Invalid request. Client is dead";
        RLOG(DEBUG) << "Req received: " << request->dateReceived() << ", client created: " << client->dateCreated() << ", age: " << client->age() << ", result: " << client->dateCreated() + client->age();
        return false;
    }

    request->setClientId(client->id());
    request->setClient(client);

    if (session != nullptr && session->client() == nullptr) {
        DRVLOG(RV_DEBUG) << "Updating session client";
        session->setClient(client);
    }

    if (!bypassChecks && client->isKnown()) {
        // take this opportunity to update the user for unknown logger

        // unknown loggers cannot be updated to specific user
        // without having a known client parent

        // make sure the current logger is unknown
        // otherwise we already know the user either from client or from logger itself
        if (m_registry->configuration()->hasFlag(Configuration::Flag::ALLOW_UNKNOWN_LOGGERS) // cannot be unknown logger unless server supports it
                && !m_registry->configuration()->isKnownLogger(request->loggerId())) {
            m_registry->configuration()->updateUnknownLoggerUserFromRequest(request->loggerId(), request);
        }
    }

    if (request->isValid()) {
        if (!bypassChecks && !isRequestAllowed(request)) {
            RLOG(WARNING) << "Ignoring log from unauthorized logger [" << request->loggerId() << "]";
            return false;
        }
        dispatch(request);
        return true;
    }
    return false;
}

void ClientQueueProcessor::dispatch(const LogRequest* request)
{
 #ifdef RESIDUE_DEV
    DRVLOG(RV_TRACE) << "Writing";
 #endif

    el::Logger* logger = el::Loggers::getLogger(request->loggerId());

    UserMessage msg(request->level(), request->filename(), request->lineNumber(), request->function(), request->verboseLevel(), logger, request);

    el::base::Writer(&msg).construct(logger) << request->msg();

 #ifdef RESIDUE_DEV
    DRVLOG(RV_TRACE) << "Write complete";
 #endif
}

bool ClientQueueProcessor::isRequestAllowed(const LogRequest* request) const
{
    Client* client = request->client();
    if (client == nullptr) {
        RLOG(DEBUG) << "Client may have expired";
        return false;
    }
    // Ensure flag is on
    bool allowed = m_registry->configuration()->hasFlag(Configuration::Flag::ALLOW_UNKNOWN_LOGGERS);
    if (!allowed) {
        // we're not allowed to use unknown loggers. we make sure the current logger is actually known.
        allowed = m_registry->configuration()->isKnownLogger(request->loggerId());
    }
    if (allowed) {
         // We do not allow users to log using residue internal logger
        allowed = request->loggerId() != RESIDUE_LOGGER_ID;
    }
    if (allowed) {
         // Logger is blacklisted
        allowed = !m_registry->configuration()->isBlacklisted(request->loggerId());
    }
    if (allowed && !client->isKnown()
            && m_registry->configuration()->isKnownLogger(request->loggerId())
            && request->loggerId() != "default") {
        allowed = false;
        DRVLOG(RV_WARNING) << "Unknown client trying to use known logger is no longer allowed";
    }
    return allowed;
}
