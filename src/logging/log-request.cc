//
//  log-request.cc
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

#include <ctime>
#include "logging/log-request.h"

using namespace residue;

LogRequest::LogRequest(const Configuration* conf) :
    Request(conf),
    m_isValid(true)
{
}

bool LogRequest::deserialize(std::string&& json)
{
    if (Request::deserialize(std::move(json))) {

        m_clientId = m_jsonDoc.get<std::string>("client_id", "");
        m_datetime = m_jsonDoc.get<unsigned long>("datetime", 0UL);
        m_token = m_jsonDoc.get<std::string>("token", "");
        m_loggerId = m_jsonDoc.get<std::string>("logger", "default");
        m_filename = m_jsonDoc.get<std::string>("file", "");
        m_function = m_jsonDoc.get<std::string>("func", "");
        m_threadId = m_jsonDoc.get<std::string>("thread", "");
        m_msg = m_jsonDoc.get<std::string>("msg", "");
        m_applicationName = m_jsonDoc.get<std::string>("app", "");
        m_level = el::LevelHelper::castFromInt(static_cast<el::base::type::EnumType>(m_jsonDoc.get<unsigned int>("level", static_cast<unsigned int>(el::Level::Unknown))));
        m_verboseLevel = static_cast<el::base::type::VerboseLevel>(m_jsonDoc.get<unsigned int>("vlevel", static_cast<unsigned int>(9)));
        m_lineNumber = static_cast<el::base::type::LineNumber>(m_jsonDoc.get<unsigned int>("line", static_cast<unsigned int>(0)));

        m_isValid = m_datetime != 0L && m_level != el::Level::Unknown && !m_loggerId.empty() && !m_msg.empty();
    }
    return m_isValid;
}

bool LogRequest::validateTimestamp() const
{
    // always valid for log request
    return true;
}

std::string LogRequest::formattedDatetime(const char* format, const el::base::MillisecondsWidth* msWidth) const
{
    struct timeval tval;
    types::TimeMs epochInMs = datetime();
    tval.tv_sec = epochInMs / 1000;
    tval.tv_usec = epochInMs % 1000000;
    return el::base::utils::DateTime::timevalToString(tval, format, msWidth);
}
