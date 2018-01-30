//
//  token-request.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include "include/log.h"
#include "src/tokenization/token-request.h"
#include "src/core/json-object.h"

using namespace residue;

TokenRequest::TokenRequest(const Configuration* conf) :
    Request(conf),
    m_isValid(true)
{
}

bool TokenRequest::deserialize(std::string&& json)
{
    if (Request::deserialize(std::move(json))) {
        m_loggerId = m_jsonObject.getString("logger_id", "");
        m_accessCode = m_jsonObject.getString("access_code", "");
        m_token = m_jsonObject.getString("token", "");
    }
    m_isValid = !m_loggerId.empty();
    return m_isValid;
}