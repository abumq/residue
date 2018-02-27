//
//  client-queue-processor.h
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

#ifndef ClientQueueProcessor_h
#define ClientQueueProcessor_h

#include <atomic>
#include <thread>
#include <string>
#include "core/request-handler.h"
#include "logging/logging-queue.h"
#include "core/json-doc.h"

namespace residue {

class LogRequest;
class Configuration;
class Registry;

///
/// \brief Responsible to process queue for client.
///
/// Each client has it's own queue that has it's own worker so other clients
/// do not get blocked.
///
/// @since 1.5.1
///
class ClientQueueProcessor final : public RequestHandler
{
public:
    ClientQueueProcessor(Registry* registry, const std::string& clientId);
    ~ClientQueueProcessor();

    inline virtual void handle(RawRequest&& logRequest) override
    {
        m_queue.push(std::move(logRequest));
    }

    ///
    /// \breif Starts the background worker
    ///
    void start();

    bool isRequestAllowed(const LogRequest*) const;
private:
    std::string m_clientId;
    std::atomic<bool> m_stopped;
    LoggingQueue m_queue;
    std::thread m_worker;
    JsonDoc m_jsonDocForBulk;

    ////
    /// \brief Dispatches the request using custom user message
    ///
    void dispatch(const LogRequest* request);

    ///
    /// \brief Parses raw request and pushes dispatches it
    ///
    void processRequestQueue();

    ///
    /// \brief Processes single log request
    /// \param clientRef A client reference pointer for fast processing (by skipping upcoming items in the bulk)
    /// \param forceCheck Whether to forcefully check the relevant properties of the request e.g, token etc.
    /// \param session Original session of the request
    /// \return True if successfully processed. Also sets client reference pointer accordingly.
    ///
    bool processRequest(LogRequest*,
                        Client** clientRef,
                        bool forceCheck,
                        Session* session);

    ///
    /// \brief Checks whether token in the specified request is still valid or not
    ///
    bool isValidToken(const LogRequest*) const;

};
}
#endif /* ClientQueueProcessor_h */
