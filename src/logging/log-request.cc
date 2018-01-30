//
//  log-request.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include <ctime>
#include "src/logging/log-request.h"
#include "src/core/json-object.h"

using namespace residue;

LogRequest::LogRequest(const Configuration* conf) :
    Request(conf),
    m_msg(LogRequestFieldMessage.defaultValue),
    m_isValid(true)
{
}

bool LogRequest::deserialize(std::string&& json)
{
    if (Request::deserialize(std::move(json))) {
        m_clientId = m_jsonObject.getString("client_id", "");

        m_datetime = resolveValue<MillisecondsEpoch>(&m_jsonObject, &LogRequestFieldDateTime);
        m_token = resolveValue<std::string>(&m_jsonObject, &LogRequestFieldToken);
        m_loggerId = resolveValue<std::string>(&m_jsonObject, &LogRequestFieldLogger);
        m_filename = resolveValue<std::string>(&m_jsonObject, &LogRequestFieldFile);
        m_function = resolveValue<std::string>(&m_jsonObject, &LogRequestFieldFunction);
        m_applicationName = resolveValue<std::string>(&m_jsonObject, &LogRequestFieldApplicationName);
        m_threadId = resolveValue<std::string>(&m_jsonObject, &LogRequestFieldThreadId);
        m_msg = resolveValue<std::string>(&m_jsonObject, &LogRequestFieldMessage);
        m_lineNumber = static_cast<el::base::type::LineNumber>(resolveValue<el::base::type::LineNumber>(&m_jsonObject, &LogRequestFieldLine));
        m_level = el::LevelHelper::castFromInt(resolveValue<el::base::type::EnumType>(&m_jsonObject, &LogRequestFieldLevel));
        m_verboseLevel = resolveValue<el::base::type::VerboseLevel>(&m_jsonObject, &LogRequestFieldVLevel);

        m_isValid = m_datetime != 0L && !m_loggerId.empty() && !m_msg.empty();
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
    const unsigned long usecOffSet = 1000000;
    unsigned long epochInMs = datetime();
    tval.tv_sec = epochInMs / 1000;
    tval.tv_usec = epochInMs % usecOffSet;
    return el::base::utils::DateTime::timevalToString(tval, format, msWidth);
}