//
//  log-request-handler.h
//  Residue
//
//  Copyright 2017-present @abumq (Majid Q.)
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

#include <string>
#include <unordered_map>

#include "core/request-handler.h"
#include "logging/client-queue-processor.h"

namespace residue {

///
/// \brief Handles incoming requests and passes it to correct queue processor
///
class LogRequestHandler final : public RequestHandler
{
public:
    LogRequestHandler(Registry*);
    ~LogRequestHandler() = default;

    ///
    /// \brief Start handling client's requests
    ///
    /// This function initiates all the client queue processors according to their respective client IDs
    ///
    void start();

    ///
    /// \brief Adds clients that are missing in existing list of processors
    ///
    void addMissingClientProcessors();

    virtual void handle(RawRequest&&);
private:
    std::unordered_map<std::string, std::unique_ptr<ClientQueueProcessor>> m_queueProcessor;

    friend class Stats;
};
}
#endif /* LogRequestHandler_h */
