//
//  log-request.h
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

#ifndef LogRequest_h
#define LogRequest_h

#include <string>

#include "core/request.h"
#include "core/types.h"
#include "logging/log.h"

namespace residue {

///
/// \brief Request to write log
///
class LogRequest final : public Request
{
public:
    explicit LogRequest(const Configuration* conf);

    std::string formattedDatetime(const char* format,
                                  const el::base::MillisecondsWidth* msWidth) const;

    inline const types::TimeMs& datetime() const
    {
        return m_datetime;
    }

    inline const el::Level& level() const
    {
        return m_level;
    }

    inline const el::base::type::string_t& msg() const
    {
        return m_msg;
    }

    inline const std::string& clientId() const
    {
        return m_clientId;
    }

    inline const std::string& loggerId() const
    {
        return m_loggerId;
    }

    inline const std::string& filename() const
    {
        return m_filename;
    }

    inline const std::string& function() const
    {
        return m_function;
    }

    inline const std::string& threadId() const
    {
        return m_threadId;
    }

    inline const std::string& threadName() const
    {
        return m_threadName;
    }

    inline const std::string& applicationName() const
    {
        return m_applicationName;
    }

    inline const el::base::type::LineNumber& lineNumber() const
    {
        return m_lineNumber;
    }

    inline const el::base::type::VerboseLevel& verboseLevel() const
    {
        return m_verboseLevel;
    }

    inline bool isValid() const
    {
        return m_isValid;
    }

    inline void setClientId(const std::string& clientId)
    {
        m_clientId = clientId;
    }

    inline bool isBulk() const
    {
        return m_jsonDoc.isArray();
    }

    virtual bool deserialize(std::string&& json) override;
    virtual bool validateTimestamp() const override;
private:

    std::string m_clientId;
    types::TimeMs m_datetime;
    std::string m_msg;
    std::string m_loggerId;
    std::string m_filename;
    std::string m_function;
    std::string m_threadId;
    std::string m_threadName;
    std::string m_applicationName;
    el::Level m_level;
    el::base::type::LineNumber m_lineNumber;
    el::base::type::VerboseLevel m_verboseLevel;
};
}
#endif /* LogRequest_h */
