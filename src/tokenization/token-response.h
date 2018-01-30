//
//  token-response.h
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

#ifndef TokenResponse_hpp
#define TokenResponse_hpp

#include <string>
#include "src/core/response.h"

namespace residue {

class Configuration;
class Client;

///
/// \brief Response for TokenRequest
///
class TokenResponse final : public Response
{
public:
    TokenResponse(Response::StatusCode, const std::string&);
    TokenResponse(const std::string&, int life,
                  const std::string& loggerId);
    TokenResponse(bool);

    void serialize(std::string& output) const;

private:
    Response::StatusCode m_status;
    std::string m_errorText;
    std::string m_token;
    std::string m_loggerId;
    int m_life;
};
}
#endif /* TokenResponse_h */
