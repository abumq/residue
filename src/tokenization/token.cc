//
//  token.cc
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

#include <chrono>
#include "src/tokenization/token.h"

using namespace residue;

Token::Token(const std::string& data, int age) :
    m_data(data),
    m_age(age)
{
    m_dateCreated = Utils::now();
}

Token::Token(const Token& other):
    m_data(other.m_data),
    m_age(other.m_age),
    m_dateCreated(other.m_dateCreated)
{
}

Token& Token::operator=(Token other)
{
    std::swap(m_data, other.m_data);
    std::swap(m_age, other.m_age);
    std::swap(m_dateCreated, other.m_dateCreated);
    return *this;
}

bool Token::operator==(const std::string& data) const
{
    return m_data == data;
}

bool Token::operator==(const Token& other) const
{
    return m_data == other.m_data;
}

bool Token::isValid(const types::Time& compareTo) const
{
    if (m_age == 0) {
        return true;
    }
    return m_dateCreated + m_age >= (compareTo == 0 ? Utils::now() : compareTo);
}
