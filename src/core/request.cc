//
//  request.cc
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
#include "src/core/request.h"
#include "src/core/configuration.h"

using namespace residue;

Request::Request(const Configuration* conf) :
    m_isValid(true),
    m_client(nullptr),
    m_configuration(conf)
{
}

bool Request::deserialize(std::string&& json)
{
    m_jsonObject = JsonObject(std::move(json));
    m_isValid = m_jsonObject.isValid();
    if (!m_isValid) {
#if RESIDUE_DEBUG
        DRVLOG(RV_ERROR) << "Malformed JSON request: " << m_jsonObject.lastError();
#else
        RVLOG(RV_ERROR) << "Malformed JSON request";
#endif
    } else {
        m_timestamp = m_jsonObject.get<types::Time>("_t", 0UL);
        m_isValid = validateTimestamp();

        RVLOG_IF(!m_isValid, RV_DEBUG) << "Potential replay. Timestamp is "
                                       << m_dateReceived << " - " << m_timestamp << " = "
                                       << (m_dateReceived - m_timestamp) << " seconds old";
    }

    return m_isValid;
}

bool Request::validateTimestamp() const
{
    if (m_timestamp != 0L) {
        return llabs(static_cast<long long>(m_dateReceived) - static_cast<long long>(m_timestamp))
                <= static_cast<long long>(m_configuration->timestampValidity());
    }
    return !m_configuration->hasFlag(Configuration::Flag::REQUIRES_TIMESTAMP);
}
