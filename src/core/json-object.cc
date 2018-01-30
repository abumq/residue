//
//  json-object.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include "include/log.h"
#include "src/core/json-object.h"

using namespace residue;

JsonObject::JsonObject()
    : m_isValid(false)
{

}

JsonObject::JsonObject(Json&& newRoot) :
    m_root(std::move(newRoot))
{

}

JsonObject::JsonObject(const Json& newRoot) :
    m_root(std::move(newRoot))
{

}

JsonObject::JsonObject(std::string&& jsonStr) :
    m_jsonStr(std::move(jsonStr))
{
    try {
        m_root = Json::parse(m_jsonStr);
        if (m_root.is_null()) {
            m_isValid = false;
            setLastError("Malformed JSON:\n" + m_jsonStr);
        } else {
            m_isValid = true;
            setLastError("");
        }
    } catch (const std::exception& e) {
        DRVLOG(RV_DEBUG) << "Error occurred while parsing JSON:" << std::endl << m_jsonStr << std::endl << "Exception: " << e.what();
        m_isValid = false;
        setLastError("Malformed JSON:\n" + m_jsonStr + "\nDetail: " + e.what());
    }
}

bool JsonObject::hasKeys(const JsonObject::Keys* keys) const
{
    for (const auto& key : *keys) {
        if (!hasKey(key)) {
            return false;
        }
    }
    return true;
}