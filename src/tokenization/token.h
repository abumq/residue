//
//  token.h
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

#ifndef Token_h
#define Token_h

#include <string>
#include "utils/utils.h"

namespace residue {

///
/// \brief Token for log request with helper functions
///
class Token final
{
public:
    Token(const std::string& data, int age = -1);
    Token(const Token&);
    Token& operator=(Token);

    inline int age() const
    {
        return m_age;
    }

    inline const std::string& data() const
    {
        return m_data;
    }

    bool isValid(const types::Time& compareTo = 0L) const;
    bool operator==(const std::string&) const;
    bool operator==(const Token&) const;
private:
    std::string m_data;
    unsigned int m_age;
    types::Time m_dateCreated;
};

using AccessCode = Token;
}

namespace std {
template<> struct hash<residue::Token>
{
public:
    std::size_t operator()(const residue::Token& t) const
    {
        return std::hash<std::string>{}(t.data());
    }
};
}

#endif /* Token_h */
