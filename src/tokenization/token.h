//
//  token.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef Token_h
#define Token_h

#include <string>
#include "src/utils/utils.h"

namespace residue {

///
/// \brief Token for log request with helper functions
///
class Token final
{
public:
    explicit Token(const std::string& data, int age = -1);
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

    bool isValid(const unsigned long& compareTo = 0L) const;
    bool operator==(const std::string&) const;
    bool operator==(const Token&) const;
private:
    std::string m_data;
    unsigned int m_age;
    unsigned long m_dateCreated;
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