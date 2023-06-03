//
//  connection-request-handler.cc
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

#include "logging/log.h"
#include "connect/connection-request-handler.h"
#include "connect/connection-request.h"
#include "connect/connection-response.h"
#include "core/client.h"
#include "core/configuration.h"
#include "net/session.h"
#include "utils/utils.h"
#include "crypto/aes.h"

using namespace residue;

ConnectionRequestHandler::ConnectionRequestHandler(Registry* registry) :
    RequestHandler("Connection", registry)
{
    DRVLOG(RV_DETAILS) << "ConnectionRequestHandler " << this << " with registry " << m_registry;
}

void ConnectionRequestHandler::handle(RawRequest&& rawRequest)
{
    ConnectionRequest request(m_registry->configuration());
    std::shared_ptr<Session> session = rawRequest.session;
    RequestHandler::handle(std::move(rawRequest), &request, Request::StatusCode::OK, true);

    if (request.keySize() != 128 && request.keySize() != 192 && request.keySize() != 256) {
        // If valid key size is specified in connection request, we use that
        // otherwise we get from configuration
        request.setKeySize(m_registry->configuration()->keySize(request.clientId()));
    }
    if (request.statusCode() != Request::StatusCode::OK) {
        ConnectionResponse response(static_cast<Response::StatusCode>(static_cast<unsigned int>(request.statusCode())),
                                    request.errorText());
        std::string output;
        response.serialize(output);
        session->write(output);
        return;
    }

    bool managedClient = !request.clientId().empty();

    if (request.type() == ConnectionRequest::Type::CONNECT && !managedClient) {
        // Initialize random client ID
        request.setClientId(Utils::generateRandomString(16));
    }

    auto respondErr = [&](const std::string& msg) {
        ConnectionResponse response(Response::StatusCode::BAD_REQUEST, msg);
        std::string output;
        response.serialize(output);
        session->write(output);
    };

    if (!request.isValid()) {
        RVLOG(RV_ERROR) << "Invalid connection request. " << request.errorText();
        respondErr("Invalid connection request. Are you sending it to correct port?");
        return;
    }

    if (request.type() == ConnectionRequest::Type::CONNECT && managedClient) {
        // Find public key for known client
        const auto& iter = m_registry->configuration()->managedClientsKeys().find(request.clientId());
        if (iter == m_registry->configuration()->managedClientsKeys().end()) {
            RLOG(ERROR) << "Client is unknown.";
            respondErr("Client is unknown");
            return;
        }
         // iter->second.first = file, iter->second.second = contents
        request.setRsaPublicKey(iter->second.second);
    } else if (request.type() == ConnectionRequest::Type::CONNECT
               && !managedClient
               && !m_registry->configuration()->hasFlag(Configuration::ALLOW_UNMANAGED_CLIENTS)) {
        RLOG(ERROR) << "Unmanaged clients are not allowed";
        respondErr("Unmanaged clients are not allowed by this server");
        return;
    }

    switch (request.type()) {
    case ConnectionRequest::Type::CONNECT:
        connect(&request, session, managedClient);
        break;
    case ConnectionRequest::Type::ACKNOWLEDGE:
        acknowledge(&request, session);
        break;
    case ConnectionRequest::Type::TOUCH:
        touch(&request, session);
        break;
    default:
        RLOG(WARNING) << "Invalid connection request type received";
    }
}

void ConnectionRequestHandler::connect(ConnectionRequest* request, const std::shared_ptr<Session>& session, bool isManagedClient) const
{
    int attempts = 0;
    while (!isManagedClient && m_registry->clientExists(request->clientId()) && attempts++ < 100) {
        // Re-generate a new client ID for this one already exists
        request->setClientId(Utils::generateRandomString(16));
    }
    if (isManagedClient && m_registry->clientExists(request->clientId())) {
        // Already connected known client, just respond with key
        Client* client = m_registry->findClient(request->clientId());
        if (!client->isAlive()) {
            // reset key
            RLOG(INFO) << "Client [" << client->id() << "] key reset";
            client->setBackupKey(client->key());
            client->setKey(AES::generateKey(request->keySize()));
            client->setKeySize(request->keySize() / 8);
        }
        // Clone client
        Client clonedClient(request);
        clonedClient.setAcknowledged(false);
        clonedClient.setKey(client->key());
        clonedClient.setKeySize(client->keySize());
        // To reduce size of the data
        clonedClient.setAge(0);
        clonedClient.setDateCreated(0);
        clonedClient.removeToken();
        ConnectionResponse response(&clonedClient);
        std::string output;
        response.serialize(output);
        session->write(output.c_str(), output.size(), client->rsaPublicKey().c_str());
    } else {
        Client client(request);
        client.setIsManaged(isManagedClient);
        client.setAge(m_registry->configuration()->nonAcknowledgedClientAge());
        if (m_registry->addClient(client)) {
            RVLOG(RV_DETAILS) << "Connected client [" << client.id() << "]";
            ConnectionResponse response(&client);
            // To reduce size of data
            response.m_clientDateCreated = 0;
            response.m_clientAge = 0;
            std::string output;
            response.serialize(output);
            session->write(output.c_str(), output.size(), client.rsaPublicKey().c_str());
        } else {
            ConnectionResponse response(Response::StatusCode::BAD_REQUEST, "Failed! Potentially duplicate client ID, please ACKNOWLEDGE.");
            std::string output;
            response.serialize(output);
            session->write(output.c_str(), output.size(), client.rsaPublicKey().c_str());
        }
    }
}

void ConnectionRequestHandler::acknowledge(const ConnectionRequest* request, const std::shared_ptr<Session>& session) const
{
    Client* existingClient = m_registry->findClient(request->clientId());
    if (existingClient == nullptr) {
        ConnectionResponse response(Response::StatusCode::BAD_REQUEST, "Client with this ID does not exists. Please send CONNECT request first.");
        std::string output;
        response.serialize(output);
        session->write(output);
        return;
    } else if (!existingClient->isManaged() && existingClient->acknowledged()) {
        ConnectionResponse response(Response::StatusCode::BAD_REQUEST, "Cannot re-acknowledge unmanaged clients. Please TOUCH to refresh it.");
        std::string output;
        response.serialize(output);
        session->write(output.c_str(), output.size(), existingClient->rsaPublicKey().c_str());
        return;
    }
    RVLOG(RV_DETAILS) << "Acknowledging client [" << existingClient->id() << "]";

    existingClient->setAcknowledged(true);
    existingClient->setAge(m_registry->configuration()->clientAge());
    existingClient->resetDateCreated();
    if (m_registry->updateClient(*existingClient)) {
        ConnectionResponse response(existingClient, m_registry->configuration());
        response.setLoggingPort(m_registry->configuration()->loggingPort());
        std::string output;
        response.serialize(output);
        session->write(output.c_str(), existingClient->key().c_str());
        session->setClient(existingClient);
    }
}

void ConnectionRequestHandler::touch(const ConnectionRequest* request, const std::shared_ptr<Session>& session) const
{
    Client* client = m_registry->findClient(request->clientId());
    if (client != nullptr) {
        RVLOG(RV_DETAILS) << "Touching client [" << client->id() << "]";
        if (!client->acknowledged()) {
            RVLOG(RV_ERROR) << "Cannot touch a non-acknowledged client";
            ConnectionResponse response(Response::StatusCode::BAD_REQUEST,
                                        "Cannot touch a non-acknowledged client. Please ACKNOWLEDGE it first");
            std::string output;
            response.serialize(output);
            session->write(output.c_str(), client->key().c_str());
        } else if (!client->isAlive()) {
            RVLOG(RV_ERROR) << "Cannot touch a dead client";
            ConnectionResponse response(Response::StatusCode::BAD_REQUEST,
                                        "Cannot touch dead client. Please reset the connection");
            std::string output;
            response.serialize(output);
            session->write(output.c_str(), client->key().c_str());
        } else {
            client->setAge(m_registry->configuration()->clientAge());
            client->resetDateCreated();
            m_registry->updateClient(*client);
            ConnectionResponse response(client, m_registry->configuration());
            response.setLoggingPort(m_registry->configuration()->loggingPort());
            std::string output;
            response.serialize(output);
            session->write(output.c_str(), client->key().c_str());
        }
    } else {
        RVLOG(RV_ERROR) << "Client is not connected. It may have died or was never connected.";
        session->writeStandardResponse(Response::StatusCode::BAD_REQUEST);
    }
}
