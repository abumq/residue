//
//  token-response.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include "src/tokenization/token-response.h"

using namespace residue;

TokenResponse::TokenResponse(Response::StatusCode status,
                             const std::string& errorText) :
    Response(),
    m_status(status),
    m_errorText(errorText),
    m_life(-1)
{

}

TokenResponse::TokenResponse(const std::string& token,
                             int life,
                             const std::string& loggerId) :
    Response(),
    m_status(StatusCode::STATUS_OK),
    m_token(token),
    m_loggerId(loggerId),
    m_life(life)
{

}

TokenResponse::TokenResponse(bool) :
    Response(),
    m_status(StatusCode::STATUS_OK),
    m_life(-1)
{

}

void TokenResponse::serialize(std::string& output) const
{
    JsonObject::Json root;
    root["status"] = m_status;
    if (!m_errorText.empty()) {
        root["error_text"] = m_errorText;
    }
    if (m_life >= 0) {
        root["life"] = m_life;
    }
    if (!m_token.empty()) {
        root["token"] = m_token;
    }
    if (!m_loggerId.empty()) {
        root["loggerId"] = m_loggerId;
    }
    Response::serialize(root, output);
}