//
//  Adminrequest.h
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

#ifndef AdminRequest_h
#define AdminRequest_h

#include <string>
#include <set>
#include "src/core/request.h"

namespace residue {

///
/// \brief Request entity for admin task
///
class AdminRequest final : public Request
{
public:
    enum class Type : unsigned short
    {
        UNKNOWN = 0,
        RELOAD_CONFIG = 1,
        ADD_CLIENT = 2,
        REMOVE_CLIENT = 3,
        RESET = 4,
        LIST_LOGGING_FILES = 5,
        FORCE_LOG_ROTATION = 6
    };

    AdminRequest(const Configuration* conf);

    inline Type type() const
    {
        return m_type;
    }

    inline std::string clientId() const
    {
        return m_clientId;
    }

    inline std::string rsaPublicKey() const
    {
        return m_rsaPublicKey;
    }

    inline std::string loggerId() const
    {
        return m_loggerId;
    }

    inline const std::set<std::string>& loggingLevels() const
    {
        return m_loggingLevels;
    }

    bool deserialize(std::string&& json);
    virtual bool validateTimestamp() const override;
private:

    std::string m_clientId;
    std::string m_rsaPublicKey;
    std::string m_loggerId;
    std::set<std::string> m_loggingLevels;
    Type m_type;
};
}
#endif /* AdminRequest_h */
