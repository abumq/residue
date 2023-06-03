//
//  request.h
//  Residue
//
//  Copyright 2017-present @abumq (Majid Q.)
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

#include "core/json-doc.h"
#include "core/types.h"
#include "non-copyable.h"

namespace residue {

class Client;
class RequestHandler;
class Configuration;

///
/// \brief Base request class
///
class Request : NonCopyable
{
public:
    enum StatusCode : unsigned short
    {
        OK = 0,
        BAD_REQUEST = 1
    };

    explicit Request(const Configuration* conf);
    virtual ~Request() = default;

    inline bool isValid() const
    {
        return m_isValid;
    }

    virtual bool deserialize(std::string&& json);

    inline const JsonDoc& jsonObject() const
    {
        return m_jsonDoc;
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

    inline const std::string& sessionId() const
    {
        return m_sessionId;
    }

    inline const types::Time& dateReceived() const
    {
        return m_dateReceived;
    }

    inline void setIpAddr(const std::string& ipAddr)
    {
        m_ipAddr = ipAddr;
    }

    inline void setSessionId(const std::string& sessionId)
    {
        m_sessionId = sessionId;
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
    JsonDoc m_jsonDoc;
    bool m_isValid;

    Client* m_client;
    std::string m_errorText;
    StatusCode m_statusCode;
    std::string m_ipAddr;
    std::string m_sessionId;

    types::Time m_timestamp;

    types::Time m_dateReceived;

    const Configuration* m_configuration;

    friend class RequestHandler;
};
}

#endif /* Request_h */
