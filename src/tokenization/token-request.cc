//
//  token-request.cc
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

#include "logging/log.h"
#include "tokenization/token-request.h"
#include "core/json-document.h"
#include "core/json-doc.h"

using namespace residue;

TokenRequest::TokenRequest(const Configuration* conf) :
    Request(conf),
    m_isValid(true)
{
}

bool TokenRequest::deserialize(std::string&& json)
{
    if (Request::deserialize(std::move(json))) {
#ifdef RESIDUE_USE_GASON
        m_loggerId = m_raw.get<std::string>("logger_id", "");
        m_accessCode = m_raw.get<std::string>("access_code", "");
        m_token = m_raw.get<std::string>("token", "");
#else
        m_loggerId = m_jsonDoc.getString("logger_id", "");
        m_accessCode = m_jsonDoc.getString("access_code", "");
        m_token = m_jsonDoc.getString("token", "");
#endif
    }
    m_isValid = !m_loggerId.empty();
    return m_isValid;
}
