//
//  admin-request.cc
//  Residue
//
//  Copyright 2017-present Zuhd Web Services
//  https://zuhd.org
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

#include "admin/admin-request.h"

#include "logging/log.h"
#include "utils/utils.h"

using namespace residue;

AdminRequest::AdminRequest(const Configuration* conf) :
    Request(conf),
    m_type(AdminRequest::Type::UNKNOWN)
{
}

bool AdminRequest::deserialize(std::string&& json)
{
    if (!Request::deserialize(std::move(json))) {
        RLOG(ERROR) << "Unable to deserialize admin request";
        return false;
    }

    m_type = static_cast<AdminRequest::Type>(m_jsonDoc.get<unsigned int>("type", 0));
    m_clientId = m_jsonDoc.get<std::string>("client_id", "");
    m_loggerId = m_jsonDoc.get<std::string>("logger_id", "");
    JsonDoc::Value levels = m_jsonDoc.get<JsonDoc::Value>("logging_levels", JsonDoc::Value());
    if (levels.isArray()) {
        for (auto level : levels) {
            m_loggingLevels.insert(level->value.toString());
        }
    }

    m_isValid = (m_type == AdminRequest::Type::RELOAD_CONFIG)
            || (m_type == AdminRequest::Type::RESET)
            || (m_type == AdminRequest::Type::LIST_LOGGING_FILES && (!m_loggerId.empty() || !m_clientId.empty()))
            || (m_type == AdminRequest::Type::FORCE_LOG_ROTATION && !m_loggerId.empty())
            || (m_type == AdminRequest::Type::STATS)
            || (m_type == AdminRequest::Type::LIST_CLIENTS);
    return m_isValid;
}


bool AdminRequest::validateTimestamp() const
{
    if (m_timestamp != 0L) {
        return llabs(static_cast<long long>(m_dateReceived) - static_cast<long long>(m_timestamp)) <= 30LL;
    }
    // we never ignore admin requests and having time stamp is a MUST
    return false;
}
