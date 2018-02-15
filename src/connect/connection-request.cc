//
//  connection-request.cc
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
#include "connect/connection-request.h"
#include "core/json-document.h"
#include "core/json-doc.h"
#include "crypto/base64.h"
#include "utils/utils.h"

using namespace residue;

ConnectionRequest::ConnectionRequest(const Configuration* conf) :
    Request(conf),
    m_type(ConnectionRequest::Type::UNKNOWN)
{
}

bool ConnectionRequest::deserialize(std::string&& json)
{
    m_isValid = true;

    if (Request::deserialize(std::move(json))) {
#ifdef RESIDUE_USE_GASON
        m_clientId = m_raw.get<std::string>("client_id", "");
        m_rsaPublicKey = Base64::decode(m_raw.get<std::string>("rsa_public_key", ""));
        m_type = static_cast<ConnectionRequest::Type>(m_raw.get<unsigned int>("type", 0));
        unsigned int keySize = m_raw.get<unsigned int>("key_size", 0);
#else
        m_clientId = m_jsonDoc.getString("client_id");
        m_rsaPublicKey = Base64::decode(m_jsonDoc.getString("rsa_public_key"));
        m_type = static_cast<ConnectionRequest::Type>(m_jsonDoc.getUInt("type", 0));
        unsigned int keySize = m_jsonDoc.getUInt("key_size", 0);
#endif

        if (keySize == 0 || keySize == 128 || keySize == 192 || keySize == 256) {
            m_keySize = keySize;
        } else {
            RLOG(ERROR) << "Invalid key size [" << keySize << "]";
            m_isValid = false;
        }
    }
    bool validConnect = (m_type == ConnectionRequest::Type::CONNECT && (!m_rsaPublicKey.empty() || !m_clientId.empty()));
    bool validSubsequentRequests = (m_type == ConnectionRequest::Type::ACKNOWLEDGE
                                    || m_type == ConnectionRequest::Type::TOUCH) && !m_clientId.empty();
    m_isValid &= validConnect || validSubsequentRequests;
    return m_isValid;
}
