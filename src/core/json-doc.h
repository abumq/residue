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

#include <memory>
#include <sstream>

#include "gason/gason.h"

namespace residue {

///
/// \brief JSON document that holds value for JSON
///
/// This is essentially a wrapper for gason
///
class JsonDoc final
{
public:
    ///
    /// \brief A single node in linked-list
    ///
    using Node = gason::JsonNode;

    ///
    /// \brief Value can hold either a whole document or a single value
    ///
    using Value = gason::JsonValue;

    ///
    /// \brief Status of JSON doc when it was parsed
    ///
    using Status = gason::JsonParseStatus;

    ///
    /// \brief Construct empty json doc with allocation failure status
    ///
    JsonDoc();

    ///
    /// \brief Construct document by node
    ///
    explicit JsonDoc(const Node* v)
        : JsonDoc()
    {
        set(v);
    }

    ///
    /// \brief Construct document by value (implicit constructor)
    ///
    JsonDoc(const Value& v)
        : JsonDoc()
    {
        set(v);
    }

    ///
    /// \brief Construct document by JSON string (calls parse(const std::string&))
    ///
    explicit JsonDoc(const std::string& json)
        : JsonDoc()
    {
        parse(json);
    }

    // disabled copying and moving

    explicit JsonDoc(const JsonDoc&) = delete;
    explicit JsonDoc(JsonDoc&&) = delete;
    JsonDoc& operator=(const JsonDoc&) = delete;
    JsonDoc& operator=(JsonDoc&&) = delete;

    ///
    /// \brief Parses JSON string and store it. Any previous parsed document for this object
    /// will be discarded
    ///
    void parse(const std::string& jstr);

    ///
    /// \brief Set value by node
    ///
    inline void set(const Node* v)
    {
        m_val = v->value;
    }

    ///
    /// \brief Set value by JsonValue
    ///
    inline void set(const Value& v)
    {
        m_val = v;
    }

    inline bool isValid() const
    {
        return m_status == gason::JsonParseStatus::JSON_PARSE_OK;
    }

    gason::JsonIterator begin() const
    {
        return gason::begin(m_val);
    }

    gason::JsonIterator end() const
    {
        return gason::end(m_val);
    }

    ///
    /// \brief Error text if parsing failed
    ///
    std::string errorText() const;

    inline bool hasKey(const char* key) const
    {
        return m_val(key).getTag() <= 5;
    }

    ///
    /// \return Whether the document base is an array or not
    ///
    inline bool isArray() const
    {
        return m_val.isArray();
    }

    ///
    /// \brief Get value by type. Please checkout out other template specializations
    /// for a specific type
    ///
    template <typename T>
    inline T get(const char* key, const T& defaultVal) const
    {
        if (isArray()) {
            return defaultVal;
        }
        return m_val(key);
    }

    ///
    /// \brief Get value as specific type. Best for single values as <code>get</code> will
    /// not work for single value
    ///
    template <typename T>
    inline T as(const T&) const
    {
        return m_val;
    }

    ///
    /// \brief Get JSON obj. Use it to create JsonDoc
    /// e.g, <pre>
    ///    JsonDoc jNew(j.getObj("blah"));
    /// </pre>
    ///
    inline JsonDoc::Value getObj(const char* key) const
    {
        return get<JsonDoc::Value>(key, JsonDoc::Value());
    }

    ///
    /// \brief Get JSON array. Use it to create JsonDoc
    /// e.g, <pre>
    ///    JsonDoc jNew(j.getArr("blah"));
    /// </pre>
    ///
    inline JsonDoc::Value getArr(const char* key) const
    {
        return getObj(key);
    }

    ///
    /// \brief Dumps JSON document as string with specific indentation
    ///
    std::string dump(int indent = -1) const;

    inline Value underlyingVal()
    {
        return m_val;
    }
private:
    Status m_status;
    Value m_val;
    gason::JsonAllocator m_alloc;
    std::unique_ptr<char[]> m_src;

    static void dump(Value o, std::stringstream& ss, int indent = -1, int depth = 1);
    static void dumpStr(const char* s, std::stringstream& ss);
};

///
/// \brief Get single value as string
/// \param defaultVal If value not found or is not a string the default value that should be returned
///
template <>
inline std::string JsonDoc::as<std::string>(const std::string& defaultVal) const
{
    if (isArray()) {
        return defaultVal;
    }
    if (m_val.isString()) {
        return m_val.toString();
    }
    return defaultVal;
}

///
/// \brief Get value as bool
/// \param key JSON key for the property
/// \param defaultVal If value not found or is not a bool the default value that should be returned
///
template <>
inline bool JsonDoc::get<bool>(const char* key, const bool& defaultVal) const
{
    if (isArray()) {
        return defaultVal;
    }
    JsonDoc::Value v = m_val(key);
    if (v.isBoolean()) {
        return v.toBool();
    }
    return defaultVal;
}

///
/// \brief Get value as string
/// \param key JSON key for the property
/// \param defaultVal If value not found or is not a string the default value that should be returned
///
template <>
inline std::string JsonDoc::get<std::string>(const char* key, const std::string& defaultVal) const
{
    if (isArray()) {
        return defaultVal;
    }
    JsonDoc::Value v = m_val(key);
    if (v.isString()) {
        return v.toString();
    }
    return defaultVal;
}

///
/// \brief Get value as int
/// \param key JSON key for the property
/// \param defaultVal If value not found or is not a number the default value that should be returned
///
template <>
inline int JsonDoc::get<int>(const char* key, const int& defaultVal) const
{
    if (isArray()) {
        return defaultVal;
    }
    JsonDoc::Value v = m_val(key);
    if (v.isNumber()) {
        return v.toInt();
    }
    return defaultVal;
}

///
/// \brief Get value as unsigned int
/// \param key JSON key for the property
/// \param defaultVal If value not found or is not a number the default value that should be returned
///
template <>
inline unsigned int JsonDoc::get<unsigned int>(const char* key, const unsigned int& defaultVal) const
{
    if (isArray()) {
        return defaultVal;
    }
    JsonDoc::Value v = m_val(key);
    if (v.isNumber()) {
        return static_cast<unsigned int>(v.toInt());
    }
    return defaultVal;
}

///
/// \brief Get value as float
/// \param key JSON key for the property
/// \param defaultVal If value not found or is not a number the default value that should be returned
///
template <>
inline float JsonDoc::get<float>(const char* key, const float& defaultVal) const
{
    if (isArray()) {
        return defaultVal;
    }
    JsonDoc::Value v = m_val(key);
    if (v.isNumber()) {
        return static_cast<float>(v.toNumber());
    }
    return defaultVal;
}

///
/// \brief Get value as unsigned long
/// \param key JSON key for the property
/// \param defaultVal If value not found or is not a number the default value that should be returned
///
template <>
inline unsigned long JsonDoc::get<unsigned long>(const char* key, const unsigned long& defaultVal) const
{
    if (isArray()) {
        return defaultVal;
    }
    JsonDoc::Value v = m_val(key);
    if (v.isNumber()) {
        return static_cast<unsigned long>(v.toNumber());
    }
    return defaultVal;
}
}

#endif /* JsonDoc_h */
