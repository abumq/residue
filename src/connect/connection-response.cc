//
//  connection-response.cc
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

#include "connect/connection-response.h"

#include "core/client.h"
#include "core/configuration.h"
#include "core/json-builder.h"

using namespace residue;

ConnectionResponse::ConnectionResponse(const Client* client, const Configuration* c):
    Response(),
    m_status(Response::StatusCode::OK),
    m_errorText(""),
    m_configuration(c),
    m_loggingPort(0),
    m_key(client->key()),
    m_clientId(client->id()),
    m_clientAge(client->age()),
    m_clientDateCreated(client->dateCreated()),
    m_isAcknowledged(client->acknowledged())
{

}

ConnectionResponse::ConnectionResponse(Response::StatusCode status, const std::string& errorText) :
    Response(),
    m_status(status),
    m_errorText(errorText),
    m_configuration(nullptr),
    m_loggingPort(0),
    m_clientAge(0),
    m_clientDateCreated(0),
    m_isAcknowledged(false)
{

}

void ConnectionResponse::serialize(std::string& output) const
{
    const std::size_t capacity = 1024;
    char source[capacity];

    JsonBuilder doc(source, capacity);
    DRVLOG(RV_DEBUG_2) << "Starting JSON serialization with [" << capacity << "] bytes";

    doc.startObject()
            .addValue("status", m_status)
            .addValue("ack", m_isAcknowledged ? 1 : 0);

    if (m_isAcknowledged && m_configuration != nullptr) {
        std::stringstream ss;
        ss << RESIDUE_VERSION;
#ifdef RESIDUE_DEBUG
        ss << "-debug";
#endif

        doc.addValue("flags", static_cast<std::size_t>(m_configuration->flag()))
           .addValue("max_bulk_size", static_cast<std::size_t>(m_configuration->maxItemsInBulk()))
           .startObject("server_info")
               .addValue("version", ss.str().c_str())
           .endObject();
    }

    if (!m_errorText.empty()) {
        doc.addValue("error_text", m_errorText.c_str());
    }
    if (!m_key.empty()) {
        doc.addValue("key", m_key.c_str());
    }
    if (!m_clientId.empty()) {
        doc.addValue("client_id", m_clientId.c_str());
    }
    if (m_loggingPort != 0) {
        doc.addValue("logging_port", m_loggingPort);
    }
    if (m_clientAge != 0) {
        doc.addValue("age", static_cast<std::size_t>(m_clientAge));
        if (m_clientDateCreated != 0) {
            doc.addValue("date_created", static_cast<std::size_t>(m_clientDateCreated));
        }
    }

    doc.endObject();

    output = source;
}
