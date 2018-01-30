//
//  request.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include "include/log.h"
#include "src/core/request.h"
#include "src/core/configuration.h"

using namespace residue;

Request::Request(const Configuration* conf) :
    m_isValid(true),
    m_client(nullptr),
    m_configuration(conf)
{
}

bool Request::deserialize(std::string&& json)
{
    m_jsonObject = JsonObject(std::move(json));
    m_isValid = m_jsonObject.isValid();
    if (!m_isValid) {
        RLOG(ERROR) << "Malformed JSON request";
        DRVLOG(RV_DEBUG) << m_jsonObject.lastError();
    } else {
        m_timestamp = m_jsonObject.get<unsigned long>("_t", 0UL);
        m_isValid = validateTimestamp();

        RVLOG_IF(!m_isValid, RV_DEBUG) << "Potential replay. Timestamp is "
                                       << m_dateReceived << " - " << m_timestamp << " = "
                                       << (m_dateReceived - m_timestamp) << " seconds old";
    }

    return m_isValid;
}

bool Request::validateTimestamp() const
{
    if (m_timestamp != 0L) {
        return llabs(static_cast<long long>(m_dateReceived) - static_cast<long long>(m_timestamp))
                <= static_cast<long long>(m_configuration->timestampValidity());
    }
    return !m_configuration->hasFlag(Configuration::Flag::REQUIRES_TIMESTAMP);
}