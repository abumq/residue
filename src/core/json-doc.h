//
//  json-doc.h
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

#ifndef JsonDoc_h
#define JsonDoc_h

#include <sstream>
#include <memory>
#include "gason/gason.h"
#include "gason/jsonbuilder.h"

namespace residue {

using JsonBuilder = gason::JSonBuilder;

class JsonDoc
{
public:
    using Value = gason::JsonValue;
    using Status = gason::JsonParseStatus;

    Status status;
    Value val;

    JsonDoc()
    {
        status = gason::JsonParseStatus::JSON_PARSE_ALLOCATION_FAILURE;
    }

    JsonDoc(const JsonDoc&) = delete;

    void parse(const std::string& jstr);

    std::string dump() const;
    static void dump(Value o, std::stringstream& ss);
    static void dumpStr(const char* s, std::stringstream& ss);

    inline bool isValid() const
    {
        return status == gason::JsonParseStatus::JSON_PARSE_OK;
    }

    gason::JsonIterator begin() const
    {
        return gason::begin(val);
    }

    gason::JsonIterator end() const
    {
        return gason::end(val);
    }

    std::string errorText() const;

    inline bool hasKey(const char* key) const
    {
        return val(key).getTag() <= 5;
    }

    inline bool isArray() const
    {
        return val.isArray();
    }

    template <typename T>
    inline T get(const char* key, const T& defaultVal) const
    {
        if (isArray()) {
            return defaultVal;
        }
        return val(key);
    }
private:
    gason::JsonAllocator alloc;
    std::unique_ptr<char[]> src;
};

template <>
inline bool JsonDoc::get<bool>(const char* key, const bool& defaultVal) const
{
    if (isArray()) {
        return defaultVal;
    }
    JsonDoc::Value v = val(key);
    if (v.isBoolean()) {
        return v.toBool();
    }
    return defaultVal;
}

template <>
inline std::string JsonDoc::get<std::string>(const char* key, const std::string& defaultVal) const
{
    if (isArray()) {
        return defaultVal;
    }
    JsonDoc::Value v = val(key);
    if (v.isString()) {
        return v.toString();
    }
    return defaultVal;
}

template <>
inline int JsonDoc::get<int>(const char* key, const int& defaultVal) const
{
    if (isArray()) {
        return defaultVal;
    }
    JsonDoc::Value v = val(key);
    if (v.isNumber()) {
        return v.toInt();
    }
    return defaultVal;
}

template <>
inline unsigned int JsonDoc::get<unsigned int>(const char* key, const unsigned int& defaultVal) const
{
    if (isArray()) {
        return defaultVal;
    }
    JsonDoc::Value v = val(key);
    if (v.isNumber()) {
        return static_cast<unsigned int>(v.toInt());
    }
    return defaultVal;
}

template <>
inline float JsonDoc::get<float>(const char* key, const float& defaultVal) const
{
    if (isArray()) {
        return defaultVal;
    }
    JsonDoc::Value v = val(key);
    if (v.isNumber()) {
        return static_cast<float>(v.toNumber());
    }
    return defaultVal;
}

template <>
inline unsigned long JsonDoc::get<unsigned long>(const char* key, const unsigned long& defaultVal) const
{
    if (isArray()) {
        return defaultVal;
    }
    JsonDoc::Value v = val(key);
    if (v.isNumber()) {
        return static_cast<unsigned long>(v.toNumber());
    }
    return defaultVal;
}
}

#endif /* JsonDoc_h */
