//
//  admin-request.cc
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
#include "admin/admin-request.h"
#include "core/json-object.h"
#include "utils/utils.h"

using namespace residue;

AdminRequest::AdminRequest(const Configuration* conf) :
    Request(conf),
    m_type(AdminRequest::Type::UNKNOWN)
{
}

Request::DeserializedObject AdminRequest::deserialize(std::string&& json)
{
    Request::DeserializedObject deserializedObj = Request::deserialize(std::move(json));
    if (!deserializedObj.isValid) {
        RLOG(ERROR) << "Unable to read request";
        return deserializedObj;
    }
    m_type = static_cast<AdminRequest::Type>(deserializedObj.jsonObject.getUInt("type", 0));
    m_clientId = deserializedObj.jsonObject.getString("client_id", "");
    m_rsaPublicKey = deserializedObj.jsonObject.getString("rsa_public_key", "");
    m_loggerId = deserializedObj.jsonObject.getString("logger_id", "");
    m_loggingLevels = deserializedObj.jsonObject.get<std::set<std::string>>("logging_levels", std::set<std::string>());

    deserializedObj.isValid = (m_type == AdminRequest::Type::RELOAD_CONFIG)
            || (m_type == AdminRequest::Type::RESET)
            || (m_type == AdminRequest::Type::ADD_CLIENT && !m_clientId.empty() && !m_rsaPublicKey.empty())
            || (m_type == AdminRequest::Type::REMOVE_CLIENT && !m_clientId.empty())
            || (m_type == AdminRequest::Type::LIST_LOGGING_FILES && (!m_loggerId.empty() || !m_clientId.empty()))
            || (m_type == AdminRequest::Type::FORCE_LOG_ROTATION && !m_loggerId.empty())
            || (m_type == AdminRequest::Type::STATS)
            || (m_type == AdminRequest::Type::LIST_CLIENTS);
    m_isValid = deserializedObj.isValid;
    return deserializedObj;
}


bool AdminRequest::validateTimestamp() const
{
    if (m_timestamp != 0L) {
        return llabs(static_cast<long long>(m_dateReceived) - static_cast<long long>(m_timestamp)) <= 30LL;
    }
    // we never ignore admin requests and having time stamp is a MUST
    return false;
}
