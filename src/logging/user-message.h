//
//  user-message.h
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

#ifndef UserMessage_h
#define UserMessage_h

#include "logging/log.h"

namespace residue {

class LogRequest;

///
/// \brief Custom log message from user with extra information
///
class UserMessage : public el::LogMessage
{
public:
    UserMessage(el::Level level, const std::string& file, el::base::type::LineNumber line, const std::string& func,
               el::base::type::VerboseLevel verboseLevel, el::Logger* logger, const LogRequest* request);

    inline const LogRequest* request() const
    {
        return m_request;
    }
private:
    const LogRequest* m_request;
};
}

#endif /* UserMessage_h */
