//
//  token-request-handler.cc
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

#include <sstream>
#include "logging/log.h"
#include "tokenization/token-request-handler.h"
#include "tokenization/token-request.h"
#include "core/configuration.h"
#include "net/session.h"
#include "utils/utils.h"
#include "tokenization/token-response.h"

using namespace residue;

TokenRequestHandler::TokenRequestHandler(Registry* registry) :
    RequestHandler("Token", registry)
{
    DRVLOG(RV_DEBUG) << "TokenRequestHandler " << this << " with registry " << m_registry;
}

void TokenRequestHandler::handle(RawRequest&& rawRequest)
{
    TokenRequest request(m_registry->configuration());
    RequestHandler::handle(std::move(rawRequest), &request);

    if (request.statusCode() != Request::StatusCode::CONTINUE) {
        m_session->write(request.errorText());
        return;
    }

    Client* client = request.client();
    if (!request.isValid()) {
        RLOG(ERROR) << "Invalid token request received...";
        TokenResponse response(Response::StatusCode::BAD_REQUEST, "No logger ID provided (Malformed JSON?)");
        std::string output;
        response.serialize(output);
        if (client != nullptr) {
            m_session->write(output.c_str(), client->key().c_str());
        } else {
            m_session->write(output);
        }
        return;
    }
    if (client == nullptr) {
        DRVLOG(RV_WARNING) << "Ignoring access to null client. Unexpected behaviour!";
        return;
    }

    if (!client->isAlive()) {
        TokenResponse response(Response::StatusCode::BAD_REQUEST, "Client is dead");
        std::string output;
        response.serialize(output);
        m_session->write(output.c_str(), client->key().c_str());
        return;
    }

    if (!request.token().empty()) {
        // Token check

        bool isValid = client->isValidToken(request.loggerId(), request.token(), m_registry, request.dateReceived());
        if (!isValid) {
            DRVLOG(RV_WARNING) << "Invalid token detected.";
            // Remove invalid token so user can re-set it
            //client->removeToken(request.loggerId(), request.token());
        }
        TokenResponse response(isValid);
        std::string output;
        response.serialize(output);
        m_session->write(output.c_str(), client->key().c_str());
    } else {
        // New token for logger
        if (!m_registry->configuration()->isValidAccessCode(request.loggerId(), request.accessCode())) {
            RLOG(ERROR) << "Invalid access code [" << request.accessCode() << "]";
            TokenResponse response(Response::StatusCode::BAD_REQUEST, "Invalid access code");
            std::string output;
            response.serialize(output);
            if (client != nullptr) {
                m_session->write(output.c_str(), client->key().c_str());
            } else {
                m_session->write(output);
            }
            return;
        }
        RVLOG(RV_DETAILS) << "Token request received for logger [" << request.loggerId() << "]";

        std::string newTokenData = Utils::generateRandomInt(8);
        int lifeTime = m_registry->configuration()->getAccessCodeTokenLife(request.loggerId(), request.accessCode());
        Token t(newTokenData, lifeTime);
        client->addToken(request.loggerId(), t);
        TokenResponse response(newTokenData, lifeTime, request.loggerId());
        std::string output;
        response.serialize(output);
        m_session->write(output.c_str(), client->key().c_str());
    }

    if (m_session->client() == nullptr) {
        m_session->setClient(client);
    }
}
