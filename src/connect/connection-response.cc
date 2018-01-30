//
//  connection-response.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include "src/connect/connection-response.h"
#include "src/core/configuration.h"
#include "src/clients/client.h"

using namespace residue;

ConnectionResponse::ConnectionResponse(const Client* client, const Configuration* c):
    Response(),
    m_status(Response::StatusCode::STATUS_OK),
    m_errorText(""),
    m_configuration(c),
    m_tokenPort(0),
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
    m_tokenPort(0),
    m_loggingPort(0),
    m_clientAge(0),
    m_clientDateCreated(0),
    m_isAcknowledged(false)
{

}

void ConnectionResponse::serialize(std::string& output) const
{
    JsonObject::Json root;
    root["status"] = m_status;
    root["ack"] = m_isAcknowledged ? 1 : 0;
    if (m_isAcknowledged && m_configuration != nullptr) {
        root["flags"] = m_configuration->flag();
        root["max_bulk_size"] = m_configuration->maxItemsInBulk();

        std::map<std::string, std::string> serverInfo;
        std::stringstream ss;
        ss << RESIDUE_VERSION;
#ifdef RESIDUE_SPECIAL_EDITION
        ss << "-SE";
#endif
        serverInfo.insert(std::make_pair("version", ss.str()));
        serverInfo.insert(std::make_pair("licensee", m_configuration->license()->licensee()));
        serverInfo.insert(std::make_pair("license_expiry", m_configuration->license()->formattedExpiry()));
        serverInfo.insert(std::make_pair("licensing_authority", m_configuration->license()->issuingAuthorityId()));
        root["server_info"] = serverInfo;
    }
    if (!m_errorText.empty()) {
        root["error_text"] = m_errorText;
    }
    if (!m_key.empty()) {
        root["key"] = m_key;
    }
    if (!m_clientId.empty()) {
        root["client_id"] = m_clientId;
    }
    if (m_tokenPort != 0) {
        root["token_port"] = m_tokenPort;
    }
    if (m_loggingPort != 0) {
        root["logging_port"] = m_loggingPort;
    }
    if (m_clientAge != 0) {
        root["age"] = m_clientAge;
        if (m_clientDateCreated != 0) {
            root["date_created"] = m_clientDateCreated;
        }
    }
    Response::serialize(root, output);
}