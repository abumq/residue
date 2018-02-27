//
//  token-response.cc
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

#include "tokenization/token-response.h"

#include "logging/log.h"
#include "core/json-builder.h"

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
    m_status(StatusCode::OK),
    m_token(token),
    m_loggerId(loggerId),
    m_life(life)
{

}

TokenResponse::TokenResponse(bool) :
    Response(),
    m_status(StatusCode::OK),
    m_life(-1)
{

}

void TokenResponse::serialize(std::string& output) const
{
    const std::size_t capacity = 128;
    char source[capacity];

    JsonBuilder doc(source, capacity);
    DRVLOG(RV_DEBUG_2) << "Starting JSON serialization with [" << capacity << "] bytes";
    doc.startObject();
    doc.addValue("status", m_status);
    if (!m_errorText.empty()) {
        doc.addValue("error_text", m_errorText);
    }
    if (m_life >= 0) {
        doc.addValue("life", m_life);
    }
    if (!m_token.empty()) {
        doc.addValue("token", m_token);
    }
    if (!m_loggerId.empty()) {
        doc.addValue("loggerId", m_loggerId);
    }
    doc.endObject();
    output = source;
}
