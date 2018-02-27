//
//  log-request-handler.h
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

#ifndef LogRequestHandler_h
#define LogRequestHandler_h

#include <atomic>
#include <thread>
#include <string>
#include "core/request-handler.h"
#include "logging/logging-queue.h"

namespace residue {

class LogRequest;
class Configuration;

///
/// \brief Request handler for LogRequest
/// \see LogRequest
///
class LogRequestHandler final : public RequestHandler
{
public:
    LogRequestHandler(Registry*);
    ~LogRequestHandler();

    ///
    /// \breif Start handling client's requests
    ///
    void start();
    bool isRequestAllowed(const LogRequest*) const;

    virtual void handle(RawRequest&&);
private:

    ////
    /// \brief Dispatches the request after temp configurating some elements of easylogging++
    ///
    void dispatch(const LogRequest* request);

    ///
    /// \brief Parses raw request and pushes it to m_requests ideally
    /// in separate thread/s (m_backgroundWorkers)
    ///
    void processRequestQueue();
    bool processRequest(LogRequest*,
                        Client** clientRef,
                        bool forceCheck,
                        Session* session);

    bool isValidToken(const LogRequest*) const;

    std::atomic<bool> m_stopped;

    LoggingQueue m_queue;

    std::thread m_backgroundWorker;
};
}
#endif /* LogRequestHandler_h */
