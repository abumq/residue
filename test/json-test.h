//
//  json-test.h
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

#ifndef JSON_TEST_H
#define JSON_TEST_H

#include "test.h"
#include "../deps/gason/gason.h"

//using namespace residue;

class JsonDoc {
public:

    struct ParseResult
    {
        int status;
        JsonValue json;
    };

    JsonDoc()
    {
        m_root = parse("NULL");
    }

    explicit JsonDoc(const std::string& json)
    {
        m_root = parse(json);
    }

    explicit JsonDoc(const JsonValue& json)
    {
        m_root = { 0, json };
    }

    static ParseResult parse(const std::string& json)
    {

        JsonAllocator allocator;
        JsonValue value;
        std::unique_ptr<char[]> source(new char[json.size() + 1]);
        strcpy(source.get(), json.c_str());

        char* end = 0;

        int status = jsonParse(source.get(), &end, &value, allocator);

        return ParseResult { status, value };
    }

    static void dump(JsonValue o, std::stringstream& ss, int indent = 0)
    {
        auto addIndent = [&](int extra = 0) {
            for (auto i = 0; i < indent + extra; ++i) {
                ss << " ";
            }
        };
        switch (o.getTag()) {
        case JSON_NUMBER:
            ss << o.toNumber();
            break;
        case JSON_STRING:
            std::cout << "tostr: " << o.toString() << std::endl;
            unescapeStr(o.toString(), ss);
            break;
        case JSON_ARRAY:
            // It is not necessary to use o.toNode() to check if an array or object
            // is empty before iterating over its members, we do it here to allow
            // nicer pretty printing.
            if (!o.toNode()) {
                ss << "[]";
                break;
            }
            ss << "[";
            for (auto i : o) {
                //addIndent(SHIFT_WIDTH);

                dump(i->value, ss);
                //dumpValue(i->value, indent + SHIFT_WIDTH);
                ss << (i->next ? "," : "");
            }
            ss << "]";
            break;
        case JSON_OBJECT:
            if (!o.toNode()) {
                ss << "{}";
                break;
            }
            ss << "{";
            for (auto i : o) {
               // fprintf(stdout, "%*s", indent + SHIFT_WIDTH, "");
                unescapeStr(i->key, ss);
                ss << ": ";
                dump(i->value, ss);
                //dumpValue(i->value, indent + SHIFT_WIDTH);
                ss << (i->next ? "," : "");
            }
            ss << "}";
            break;
        case JSON_TRUE:
            ss << "true";
            break;
        case JSON_FALSE:
            ss << "false";
            break;
        case JSON_NULL:
            ss << "null";
            break;
        }
    }

    inline bool isValid() const
    {
        return m_root.status == JSON_OK;
    }

    inline std::string error() const
    {
        return jsonStrError(m_root.status);
    }

    inline bool isObject() const
    {
        return m_root.json.getTag() == JSON_OBJECT;
    }

    inline bool isArray() const
    {
        return m_root.json.getTag() == JSON_ARRAY;
    }

    template <typename T = JsonDoc>
    T get(const std::string& key)
    {
        for (auto j : m_root.json) {
            if (strcmp(j->key, key.c_str()) == 0) {
                return getValueAs<T>(j->value);
            }
        }
        return T();
    }

    inline JsonIterator begin() const
    {
        return JsonIterator { m_root.json.toNode() };
    }

    inline JsonIterator end() const
    {
        return JsonIterator { nullptr };
    }

    void dump(std::stringstream& ss, int indent = 0) const
    {
        dump(m_root.json, ss, indent);
    }

private:
    ParseResult m_root;

    template <typename T>
    T getValueAs(const JsonValue& j)
    {
        return T(j);
    }

    static void unescapeStr(const char* s, std::stringstream& ss)
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
};

template<>
std::string JsonDoc::getValueAs(const JsonValue& j)
{
    std::string r;
    r = j.toString();
    return r;
}

template<>
double JsonDoc::getValueAs(const JsonValue& j)
{
    return j.toNumber();
}

TEST(JsonTest, SimpleParse)
{

    JsonDoc t("{\"t\":0}");
    ASSERT_TRUE(t.isValid()) << t.error();
    ASSERT_TRUE(t.isObject());

    JsonDoc t2("{\"t\":[[32, 33]]}");
    ASSERT_TRUE(t2.isValid()) << t2.error();
    ASSERT_TRUE(t2.isObject());

    const std::string bigNestedArray = "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[1, 2]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]";

    JsonDoc t3("{\"t\":" + bigNestedArray + "}");
    ASSERT_FALSE(t3.isValid());
    ASSERT_EQ(t3.error(), "stack overflow");


    JsonDoc t4("{\"t\":[32, 33]}");
    ASSERT_TRUE(t4.isValid());
    ASSERT_TRUE(t4.isObject());
    JsonDoc t4_t = t4.get("t");
    ASSERT_TRUE(t4_t.isValid());
    ASSERT_TRUE(t4_t.isArray());


    JsonDoc obj("{\"people\":[{\"name\":\"adam\", \"age\": 960}, {\"name\":\"david\", \"age\": 100}]}");
    ASSERT_TRUE(obj.isValid());
    ASSERT_TRUE(obj.isObject());
    JsonDoc people = obj.get("people");
    ASSERT_TRUE(people.isValid());
    ASSERT_TRUE(people.isArray());

    for (auto person : people) {
        JsonDoc d(person->value);

        std::cout << d.get<std::string>("name") << std::endl;
        std::string id = d.get<std::string>("id");
        std::cout << id << std::endl;


    }

    std::stringstream ss;
    obj.dump(ss);
    std::cout << ss.str() << std::endl;

}

#endif // JSON_TEST_H
