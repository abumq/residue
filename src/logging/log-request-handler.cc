//
//  log-request-handler.cc
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

#include "logging/log-request-handler.h"

#include "core/configuration.h"
#include "logging/client-queue-processor.h"
#include "logging/log.h"
#include "logging/log-request.h"

using namespace residue;

LogRequestHandler::LogRequestHandler(Registry* registry) :
    RequestHandler("Log", registry)
{
    DRVLOG(RV_DEBUG) << "LogRequestHandler " << this << " with registry " << m_registry;
}

void LogRequestHandler::start()
{
    std::string clientId = Configuration::UNKNOWN_CLIENT_ID;
    ClientQueueProcessor* processor = new ClientQueueProcessor(m_registry, clientId);
    processor->start();

    m_queueProcessor.insert({ clientId, std::unique_ptr<ClientQueueProcessor>(processor) }); // for unknown

    for (auto& knownClientPair : m_registry->configuration()->knownClientsKeys()) {
        clientId = knownClientPair.first;
        processor = new ClientQueueProcessor(m_registry, clientId);
        m_queueProcessor.insert({ clientId, std::unique_ptr<ClientQueueProcessor>(processor) });
        processor->start();
    }
}

void LogRequestHandler::handle(RawRequest&& rawRequest)
{
    LogRequest request(m_registry->configuration());
    RequestHandler::handleWithCopy(rawRequest, &request, Request::StatusCode::BAD_REQUEST,
                           false, false,  m_registry->configuration()->hasFlag(Configuration::Flag::COMPRESSION));

    // bad request
    if ((!request.isValid() && !request.isBulk())
            || request.statusCode() == Request::StatusCode::BAD_REQUEST) {
        rawRequest.session->writeStandardResponse(Response::StatusCode::BAD_REQUEST);
        return;
    }

    if (request.client() == nullptr) {
        // no way we are able to process this request
        rawRequest.session->writeStandardResponse(Response::StatusCode::INVALID_CLIENT);
    } else {
        rawRequest.session->writeStandardResponse(Response::StatusCode::OK);

        // we do not queue up decrypted request here as it gets messy
        // with all the copy constructors and move constructors.
        // Processors run on different thread so it's OK to decrypt it second time
        if (!request.client()->isKnown()) {
            m_queueProcessor.find(Configuration::UNKNOWN_CLIENT_ID)->second->handle(std::move(rawRequest));
        } else {
            m_queueProcessor.find(request.client()->id())->second->handle(std::move(rawRequest));
        }
    }

}
