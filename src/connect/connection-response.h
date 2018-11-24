//
//  connection-response.h
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

#ifndef ConnectionResponse_h
#define ConnectionResponse_h

#include <string>
#include "core/response.h"
#include "connect/connection-request-handler.h"

namespace residue {

class Configuration;
class Client;
class Configuration;

///
/// \brief Response for connect request
/// \see ConnectionRequest
///
class ConnectionResponse final : public Response
{
public:
    ConnectionResponse(Response::StatusCode, const std::string&);
    ConnectionResponse(const Client*, const Configuration* c = nullptr);

    void serialize(std::string& output) const;

    inline void setLoggingPort(unsigned int loggingPort)
    {
        m_loggingPort = loggingPort;
    }

private:
    StatusCode m_status;
    std::string m_errorText;
    const Configuration* m_configuration;

    int m_loggingPort;
    std::string m_key;
    std::string m_clientId;
    std::string m_clientToken;
    unsigned int m_clientAge;
    types::Time m_clientDateCreated;
    bool m_isAcknowledged;

    friend class ConnectionRequestHandler;
};
}

#endif /* ConnectionResponse_h */
