//
//  connection-request.h
//  Residue
//
//  Copyright 2017-present Amrayn Web Services
//  https://amrayn.com
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

#ifndef ConnectionRequest_h
#define ConnectionRequest_h

#include <string>
#include "core/request.h"

namespace residue {

///
/// \brief Request entity to connect to the server. This request can be of following types:
/// Connect, Disconnect, Acknowledge, Touch
///
class ConnectionRequest final : public Request
{
public:
    enum class Type : unsigned short
    {
        UNKNOWN = 0,
        CONNECT = 1,
        ACKNOWLEDGE = 2,
        TOUCH = 3,
    };

    explicit ConnectionRequest(const Configuration* conf);

    inline const std::string& clientId() const
    {
        return m_clientId;
    }

    inline void setKeySize(unsigned int keySize)
    {
        m_keySize = keySize;
    }

    inline void setClientId(const std::string& newClientId)
    {
        m_clientId = newClientId;
    }

    inline void setRsaPublicKey(const std::string& rsaPublicKey)
    {
        m_rsaPublicKey = rsaPublicKey;
    }

    inline const std::string& rsaPublicKey() const
    {
        return m_rsaPublicKey;
    }

    inline Type type() const
    {
        return m_type;
    }

    inline unsigned int keySize() const
    {
        return m_keySize;
    }

    virtual bool deserialize(std::string&& json) override;
private:
    std::string m_clientId;
    std::string m_rsaPublicKey;
    unsigned int m_keySize;
    Type m_type;
};
}
#endif /* ConnectionRequest_h */
