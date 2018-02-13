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
#include "core/json-object.h"

using namespace residue;

TokenRequest::TokenRequest(const Configuration* conf) :
    Request(conf),
    m_isValid(true)
{
}

Request::DeserializedObject TokenRequest::deserialize(std::string&& json)
{
    Request::DeserializedObject deserializedObj = Request::deserialize(std::move(json));
    if (deserializedObj.isValid) {
        m_loggerId = deserializedObj.jsonObject.getString("logger_id", "");
        m_accessCode = deserializedObj.jsonObject.getString("access_code", "");
        m_token = deserializedObj.jsonObject.getString("token", "");
    }
    deserializedObj.isValid = !m_loggerId.empty();
    m_isValid = deserializedObj.isValid;
    return deserializedObj;
}
