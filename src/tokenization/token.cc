//
//  token.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
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

bool Token::isValid(const unsigned long& compareTo) const
{
    if (m_age == 0) {
        return true;
    }
    return m_dateCreated + m_age >= (compareTo == 0 ? Utils::now() : compareTo);
}