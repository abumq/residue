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

void JsonDoc::parse(const std::string& jstr)
{
    src = std::unique_ptr<char[]>(new char[jstr.size() + 1]);
    strcpy(src.get(), jstr.c_str());
    status = gason::jsonParse(src.get(), val, alloc);
}

std::string JsonDoc::dump(int indent) const
{
    std::stringstream ss;
    dump(val, ss, indent);
    return ss.str();
}

void JsonDoc::dump(JsonDoc::Value o, std::stringstream& ss, int indent, int depth)
{
    const std::string spaces = indent > 0 ? std::string(depth * indent, ' ') : "";
    const std::string indentBack = indent > 0 && depth > 1 ? std::string((depth - 1) * indent, ' ') : "";
    const std::string newLine = indent > 0 ? "\n" : "";
    const std::string separator = indent > 0 ? " " : "";

    switch (o.getTag()) {
    case gason::JSON_NUMBER:
        ss << static_cast<long>(o.toNumber());
        break;
    case gason::JSON_STRING:
        dumpStr(o.toString(), ss);
        break;
    case gason::JSON_ARRAY:
        if (!o.toNode()) {
            ss << "[]";
            break;
        }
        ss << "[" << newLine << spaces;
        for (auto i : o) {
            dump(i->value, ss, indent, depth + 1);
            ss << (i->next ? "," + newLine : "");
            ss << spaces;
        }
        ss << newLine << indentBack << "]";
        break;
    case gason::JSON_OBJECT:
        if (!o.toNode()) {
            ss << "{}";
            break;
        }
        ss << "{" << newLine;
        for (auto i : o) {
            ss << spaces;
            dumpStr(i->key, ss);
            ss << ":" << separator;
            dump(i->value, ss, indent, depth + 1);
            ss << (i->next ? "," + newLine : "");
        }
        ss << newLine << indentBack << "}";
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

std::string JsonDoc::errorText() const
{
    switch (status)
    {
    case gason::JsonParseStatus::JSON_PARSE_OK:
        return "";
    case gason::JsonParseStatus::JSON_PARSE_BAD_NUMBER:
        return "Bad number";
    case gason::JsonParseStatus::JSON_PARSE_ALLOCATION_FAILURE:
        return "Failed to allocate memory";
    case gason::JsonParseStatus::JSON_PARSE_BAD_STRING:
        return "Bad string";
    case gason::JsonParseStatus::JSON_PARSE_MISMATCH_BRACKET:
        return "Unclosed bracket";
    case gason::JsonParseStatus::JSON_PARSE_STACK_OVERFLOW:
        return "Too much nesting";
    case gason::JsonParseStatus::JSON_PARSE_UNEXPECTED_CHARACTER:
        return "Unexpected character";
    case gason::JsonParseStatus::JSON_PARSE_UNQUOTED_KEY:
        return "Unquoted keys are not allowed";
    default:
        return "Unknown error - [hint: single quote keys are not allowed]";
    }
}
