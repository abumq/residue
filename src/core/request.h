//
//  request.h
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

#ifndef Request_h
#define Request_h

#include <string>
#include "non-copyable.h"
#include "core/json-object.h"
#include "core/types.h"

namespace residue {

class Client;
class RequestHandler;
class Configuration;

///
/// \brief Base request class
/// \see AdminRequest
/// \see ConnectionRequest
/// \see TokenRequest
/// \see LogRequest
///
class Request : NonCopyable
{
public:
    enum StatusCode : unsigned short
    {
        STATUS_OK = 0,
        BAD_REQUEST = 1,
        CONTINUE = 0
    };

    explicit Request(const Configuration* conf);
    virtual ~Request() = default;

    inline bool isValid() const
    {
        return m_isValid;
    }

    bool deserialize(std::string&& json);

    inline JsonObject jsonObject() const
    {
        return m_jsonObject;
    }

    inline Client* client() const
    {
        return m_client;
    }

    inline const std::string& errorText() const
    {
        return m_errorText;
    }

    inline const StatusCode& statusCode() const
    {
        return m_statusCode;
    }

    inline const std::string& ipAddr() const
    {
        return m_ipAddr;
    }

    inline const types::Time& dateReceived() const
    {
        return m_dateReceived;
    }

    inline void setIpAddr(const std::string& ipAddr)
    {
        m_ipAddr = ipAddr;
    }

    inline void setDateReceived(const types::Time& dateReceived)
    {
        m_dateReceived = dateReceived;
    }

    inline void setClient(Client* client)
    {
        m_client = client;
    }

    template <class Other>
    inline bool typeOf() const
    {
        return std::is_base_of<Request, Other>::value;
    }

    virtual bool validateTimestamp() const;
protected:
    JsonObject m_jsonObject;
    bool m_isValid;

    Client* m_client;
    std::string m_errorText;
    StatusCode m_statusCode;
    std::string m_ipAddr;

    types::Time m_timestamp;

    types::Time m_dateReceived;

    const Configuration* m_configuration;

    friend class RequestHandler;
};
}

#endif /* Request_h */
