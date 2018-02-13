//
//  token-request.h
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

#ifndef TokenRequest_h
#define TokenRequest_h

#include <string>
#include "core/request.h"

namespace residue {

///
/// \brief Incoming token request
///
class TokenRequest final : public Request
{
public:
    explicit TokenRequest(const Configuration* conf);

    inline const std::string& loggerId() const
    {
        return m_loggerId;
    }

    inline const std::string& token() const
    {
        return m_token;
    }

    inline const std::string& accessCode() const
    {
        return m_accessCode;
    }

    inline bool isValid() const
    {
        return m_isValid;
    }

    virtual bool deserialize(std::string&& json) override;
private:

    std::string m_loggerId;
    std::string m_accessCode;
    std::string m_token;

    bool m_isValid;
};
}
#endif /* TokenRequest_h */
