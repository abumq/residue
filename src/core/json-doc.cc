//
//  json-doc.cc
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

#include "core/json-doc.h"

using namespace residue;

template <>
bool JsonDoc::get(const char* key, const bool& defaultVal) const
{
    JsonDoc::Value v = val(key);
    if (v.isBoolean()) {
        return v.toBool();
    }
    return defaultVal;
}

template <>
std::string JsonDoc::get(const char* key, const std::string& defaultVal) const
{
    JsonDoc::Value v = val(key);
    if (v.isString()) {
        return v.toString();
    }
    return defaultVal;
}

template <>
int JsonDoc::get(const char* key, const int& defaultVal) const
{
    JsonDoc::Value v = val(key);
    if (v.isNumber()) {
        return v.toInt();
    }
    return defaultVal;
}

template <>
unsigned int JsonDoc::get(const char* key, const unsigned int& defaultVal) const
{
    JsonDoc::Value v = val(key);
    if (v.isNumber()) {
        return static_cast<unsigned int>(v.toInt());
    }
    return defaultVal;
}

template <>
float JsonDoc::get(const char* key, const float& defaultVal) const
{
    JsonDoc::Value v = val(key);
    if (v.isNumber()) {
        return static_cast<float>(v.toNumber());
    }
    return defaultVal;
}

void JsonDoc::parse(const std::string& jstr)
{
    std::unique_ptr<char[]> buf(new char[jstr.size() + 1]);
    strcpy(buf.get(), jstr.c_str());
    status = gason::jsonParse(buf.get(), val, alloc);
}

std::string JsonDoc::dump() const
{
    std::stringstream ss;
    dump(val, ss);
    return ss.str();
}

void JsonDoc::dump(JsonDoc::Value o, std::stringstream& ss)
{
    switch (o.getTag()) {
    case gason::JSON_NUMBER:
        ss << o.toNumber();
        break;
    case gason::JSON_STRING:
        dumpStr(o.toString(), ss);
        break;
    case gason::JSON_ARRAY:
        if (!o.toNode()) {
            ss << "[]";
            break;
        }
        ss << "[";
        for (auto i : o) {
            dump(i->value, ss);
            ss << (i->next ? "," : "");
        }
        ss << "]";
        break;
    case gason::JSON_OBJECT:
        if (!o.toNode()) {
            ss << "{}";
            break;
        }
        ss << "{";
        for (auto i : o) {
            dumpStr(i->key, ss);
            ss << ":";
            dump(i->value, ss);
            ss << (i->next ? "," : "");
        }
        ss << "}";
        break;
    case gason::JSON_TRUE:
        ss << "true";
        break;
    case gason::JSON_FALSE:
        ss << "false";
        break;
    case gason::JSON_NULL:
        ss << "null";
        break;
    }
}

void JsonDoc::dumpStr(const char *s, std::stringstream &ss)
{
    ss << "\"";
    while (*s) {
        int c = *s++;
        switch (c) {
        case '\b':
            ss << "\\b";
            break;
        case '\f':
            ss << "\\f";
            break;
        case '\n':
            ss << "\\n";
            break;
        case '\r':
            ss << "\\r";
            break;
        case '\t':
            ss << "\\t";
            break;
        case '\\':
            ss << "\\\\";
            break;
        case '"':
            ss << "\\\"";
            break;
        default:
            ss << (char) c;
        }
    }
    ss << "\"";
}
