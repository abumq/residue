//
//  json-document.cc
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

#include "logging/log.h"
#include "core/json-document.h"

using namespace residue;

JsonDocument::JsonDocument()
    : m_isValid(false)
{

}

JsonDocument::JsonDocument(JsonItem&& newRoot)
    : m_root(std::move(newRoot))
{
}

JsonDocument::JsonDocument(const JsonItem& newRoot)
    : m_root(std::move(newRoot))
{
}

JsonDocument::JsonDocument(std::string&& jsonStr) :
    m_jsonStr(std::move(jsonStr))
{
    try {
        m_root = JsonItem::parse(m_jsonStr);
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

bool JsonDocument::hasKeys(const JsonDocument::Keys* keys) const
{
    for (const auto& key : *keys) {
        if (!hasKey(key)) {
            return false;
        }
    }
    return true;
}
