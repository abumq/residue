//
//  user-message.cc
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

#include "logging/user-message.h"

#include "logging/log-request.h"

using namespace residue;

UserMessage::UserMessage(el::Level level,
                         const std::string &file,
                         el::base::type::LineNumber line,
                         const std::string &func,
                         el::base::type::VerboseLevel verboseLevel,
                         el::Logger *logger,
                         const LogRequest* request) :
    el::LogMessage(level, file, line, func, verboseLevel, logger),
    m_request(request)
{

}
