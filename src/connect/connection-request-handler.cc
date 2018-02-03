//
//  connection-request-handler.cc
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

#include "src/logging/log.h"
#include "src/connect/connection-request-handler.h"
#include "src/connect/connection-request.h"
#include "src/connect/connection-response.h"
#include "src/core/configuration.h"
#include "src/net/session.h"
#include "src/clients/client.h"
#include "src/utils/utils.h"

using namespace residue;

ConnectionRequestHandler::ConnectionRequestHandler(Registry* registry) :
    RequestHandler(registry)
{
    DRVLOG(RV_DETAILS) << "ConnectionRequestHandler " << this << " with registry " << m_registry;
}

void ConnectionRequestHandler::handle(RawRequest&& rawRequest)
{
    ConnectionRequest request(m_registry->configuration());
    RequestHandler::handle(std::move(rawRequest), &request, Request::StatusCode::CONTINUE, true);

    if (request.keySize() != 128 && request.keySize() != 192 && request.keySize() != 256) {
        // If valid key size is specified in connection request, we use that
        // otherwise we get from configuration
        request.setKeySize(m_registry->configuration()->keySize(request.clientId()));
    }
    if (request.statusCode() != Request::StatusCode::CONTINUE) {
        ConnectionResponse response(static_cast<Response::StatusCode>(static_cast<unsigned int>(request.statusCode())),
                                    request.errorText());
        std::string output;
        response.serialize(output);
        m_session->write(output);
        return;
    }

    bool knownClient = !request.clientId().empty();

    if (request.type() == ConnectionRequest::Type::CONNECT && !knownClient) {
        // Initialize random client ID
        request.setClientId(Utils::generateRandomString(16));
    }

    auto respondErr = [&](const std::string& msg) {
        ConnectionResponse response(Response::StatusCode::BAD_REQUEST, msg);
        std::string output;
        response.serialize(output);
        m_session->write(output);
    };

    if (!request.isValid()) {
        RVLOG(RV_ERROR) << "Invalid connection request received... (is port correct?)";
        respondErr("Invalid connection request");
        return;
    }

    if (request.type() == ConnectionRequest::Type::CONNECT && knownClient) {
        // Find public key for known client
        const auto& iter = m_registry->configuration()->knownClientsKeys().find(request.clientId());
        if (iter == m_registry->configuration()->knownClientsKeys().end()) {
            RVLOG(RV_ERROR) << "Client is unknown.";
            respondErr("Client is unknown");
            return;
        }
         // iter->second.first = file, iter->second.second = contents
        request.setRsaPublicKey(iter->second.second);
    } else if (request.type() == ConnectionRequest::Type::CONNECT
               && !knownClient
               && !m_registry->configuration()->hasFlag(Configuration::ALLOW_UNKNOWN_CLIENTS)) {
        RVLOG(RV_ERROR) << "Client unknown. Not allowed";
        respondErr("Unknown clients are not allowed");
        return;
    }

    switch (request.type()) {
    case ConnectionRequest::Type::CONNECT:
        connect(&request, knownClient);
        break;
    case ConnectionRequest::Type::ACKNOWLEDGE:
        acknowledge(&request);
        break;
    case ConnectionRequest::Type::TOUCH:
        touch(&request);
        break;
    default:
        RVLOG(RV_WARNING) << "Invalid connection request type";
    }
}

void ConnectionRequestHandler::connect(ConnectionRequest* request, bool isKnownClient) const
{
    int attempts = 0;
    while (!isKnownClient && m_registry->clientExists(request->clientId()) && attempts++ < 100) {
        // Re-generate a new client ID for this one already exists
        request->setClientId(Utils::generateRandomString(16));
    }
    if (isKnownClient && m_registry->clientExists(request->clientId())) {
        // Already connected known client, just respond with key
        Client* client = m_registry->findClient(request->clientId());
        // Clone client
        Client clonedClient(request);
        clonedClient.setAcknowledged(false);
        clonedClient.setKey(client->key());
        clonedClient.setKeySize(client->keySize());
        // To reduce size of the data
        clonedClient.setAge(0);
        clonedClient.setDateCreated(0);
        ConnectionResponse response(&clonedClient);
        std::string output;
        response.serialize(output);
        m_session->write(output.c_str(), output.size(), client->rsaPublicKey().c_str());
    } else {
        Client client(request);
        client.setIsKnown(isKnownClient);
        client.setAge(m_registry->configuration()->nonAcknowledgedClientAge());
        if (m_registry->addClient(client)) {
            RVLOG(RV_DETAILS) << "Connected client [" << client.id() << "]";
            ConnectionResponse response(&client);
            // To reduce size of data
            response.m_clientDateCreated = 0;
            response.m_clientAge = 0;
            std::string output;
            response.serialize(output);
            m_session->write(output.c_str(), output.size(), client.rsaPublicKey().c_str());
        } else {
            ConnectionResponse response(Response::StatusCode::BAD_REQUEST, "Failed! Potentially duplicate client ID, please ACKNOWLEDGE.");
            std::string output;
            response.serialize(output);
            m_session->write(output.c_str(), output.size(), client.rsaPublicKey().c_str());
        }
    }
}

void ConnectionRequestHandler::acknowledge(const ConnectionRequest* request) const
{
    Client* existingClient = m_registry->findClient(request->clientId());
    if (existingClient == nullptr) {
        ConnectionResponse response(Response::StatusCode::BAD_REQUEST, "Client with this ID does not exists. Please send CONNECT request first.");
        std::string output;
        response.serialize(output);
        m_session->write(output);
        return;
    } else if (!existingClient->isKnown() && existingClient->acknowledged()) {
        ConnectionResponse response(Response::StatusCode::BAD_REQUEST, "Cannot re-acknowledge unknown clients. Please TOUCH to refresh it.");
        std::string output;
        response.serialize(output);
        m_session->write(output.c_str(), output.size(), existingClient->rsaPublicKey().c_str());
        return;
    }
    RVLOG(RV_DETAILS) << "Acknowledging client [" << existingClient->id() << "]";

    existingClient->setAcknowledged(true);
    existingClient->setAge(m_registry->configuration()->clientAge());
    existingClient->resetDateCreated();
    if (m_registry->updateClient(*existingClient)) {
        ConnectionResponse response(existingClient, m_registry->configuration());
        response.setTokenPort(m_registry->configuration()->tokenPort());
        response.setLoggingPort(m_registry->configuration()->loggingPort());
        std::string output;
        response.serialize(output);
        m_session->write(output.c_str(), existingClient->key().c_str());
    }
}

void ConnectionRequestHandler::touch(const ConnectionRequest* request) const
{
    Client* client = m_registry->findClient(request->clientId());
    if (client != nullptr) {
        RVLOG(RV_DETAILS) << "Touching client [" << client->id() << "]";
        if (!client->acknowledged()) {
            RVLOG(RV_ERROR) << "Cannot touch a non-acknowledged client";
            m_session->writeStatusCode(Response::StatusCode::BAD_REQUEST);
        } else if (!client->isAlive()) {
            RVLOG(RV_ERROR) << "Cannot touch a dead client";
            ConnectionResponse response(Response::StatusCode::BAD_REQUEST, "Cannot touch dead client. Please reset the connection");
            std::string output;
            response.serialize(output);
            m_session->write(output.c_str(), client->key().c_str());
        } else {
            client->setAge(m_registry->configuration()->clientAge());
            client->resetDateCreated();
            m_registry->updateClient(*client);
            ConnectionResponse response(client, m_registry->configuration());
            response.setTokenPort(m_registry->configuration()->tokenPort());
            response.setLoggingPort(m_registry->configuration()->loggingPort());
            std::string output;
            response.serialize(output);
            m_session->write(output.c_str(), client->key().c_str());
        }
    } else {
        RVLOG(RV_ERROR) << "Client is not connected. It may have died or was never connected.";
        m_session->writeStatusCode(Response::StatusCode::BAD_REQUEST);
    }
}
