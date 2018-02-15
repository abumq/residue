//
//  json-object.h
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

#ifndef JsonObject_h
#define JsonObject_h

#include <iterator>
#include "nlohmann-json/json.h"
//#include "gason/gason.h"
#include "logging/log.h"
#include "non-copyable.h"

namespace residue {

///
/// \brief Json wrapper to serialize / deserialize json string
///
class JsonObject final
{
public:
    using Json = nlohmann::json;
    using Keys = std::vector<const char*>;

    JsonObject();

    explicit JsonObject(const Json& newRoot);
    explicit JsonObject(Json&& newRoot);
    explicit JsonObject(std::string&& jsonStr);

    inline bool isValid() const
    {
        return m_isValid;
    }

    inline std::string lastError() const
    {
        return m_lastError;
    }

    inline void setLastError(const std::string& lastError)
    {
        m_lastError = lastError;
    }

    inline const std::string& jsonStr() const
    {
        return m_jsonStr;
    }

    Json::const_iterator begin() const
    {
        return m_root.begin();
    }

    Json::const_iterator end() const
    {
        return m_root.end();
    }

    inline Json root(void) const
    {
        return m_root;
    }

    inline unsigned int getUInt(const std::string& key,
                                unsigned int defaultValue = 0) const
    {
        return get<unsigned int>(key, defaultValue);
    }

    inline unsigned long getULong(const std::string& key,
                                  unsigned long defaultValue = 0L) const
    {
        return get<unsigned long>(key, defaultValue);
    }

    inline std::string getString(const std::string& key,
                                 const std::string& defaultValue = "") const
    {
        return get<std::string>(key, defaultValue);
    }

    inline bool getBool(const std::string& key,
                        bool defaultValue = false) const
    {
        return get<bool>(key, defaultValue);
    }

    inline const char* getCString(const std::string& key,
                                  const char* defaultValue = "") const
    {
        return getString(key, defaultValue).c_str();
    }

    inline bool hasKey(const std::string& key) const
    {
         return m_root.count(key) > 0;
    }

    bool hasKeys(const Keys* keys) const;

    inline bool isArray() const
    {
        return m_isValid && m_root.is_array();
    }

    template <typename T>
    T get(const std::string& key, const T& defaultValue) const
    {
        if (!hasKey(key)) {
            return defaultValue;
        }
        try {
            return m_root[key];
        } catch (std::exception& e) {
            DRVLOG(RV_ERROR) << "Exception thrown when reading key " << key << std::endl << "Exception: " << e.what();
            return defaultValue;
        }
    }
private:
    std::string m_jsonStr;
    Json m_root;
    bool m_isValid;
    std::string m_lastError;
};
}

#endif /* JsonObject_h */
