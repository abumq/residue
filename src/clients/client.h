//
//  client.h
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

#ifndef Client_h
#define Client_h

#include <string>
#include <unordered_map>
#include <unordered_set>
#include "tokenization/token.h"
#include "utils/utils.h"

namespace residue {

class ConnectionRequest;
class Registry;

///
/// \brief Single client object known to the server
///
class Client
{
public:
    explicit Client(const ConnectionRequest* request);

    virtual ~Client();

    inline bool operator==(const std::string& id) const
    {
        return m_id == id;
    }

    inline bool operator==(const Client& other) const
    {
        return m_id == other.m_id;
    }

    inline const std::string& id() const
    {
        return m_id;
    }

    inline bool acknowledged() const
    {
        return m_acknowledged;
    }

    inline const std::string& rsaPublicKey() const
    {
        return m_rsaPublicKey;
    }

    inline void setRsaPublicKey(const std::string& rsaPublicKey)
    {
        m_rsaPublicKey = rsaPublicKey;
    }

    inline const std::string& key() const
    {
        return m_key;
    }

    inline const std::string& backupKey() const
    {
        return m_backupKey;
    }

    inline int keySize() const
    {
        return m_keySize;
    }

    inline void setAge(unsigned int age)
    {
        m_age = age;
    }

    inline unsigned int age() const
    {
        return m_age;
    }

    inline void setDateCreated(types::Time dateCreated)
    {
        m_dateCreated = dateCreated;
    }

    inline types::Time dateCreated() const
    {
        return m_dateCreated;
    }

    inline void resetDateCreated()
    {
        m_dateCreated = Utils::now();
    }

    inline void setAcknowledged(bool acknowledged)
    {
        m_acknowledged = acknowledged;
    }

    inline void setIsKnown(bool isKnown)
    {
        m_isKnown = isKnown;
    }

    inline bool isKnown() const
    {
        return m_isKnown;
    }

    inline void setKeySize(int keySize)
    {
        m_keySize = keySize;
    }

    inline void setKey(const std::string& key)
    {
        m_key = key;
    }

    inline void setBackupKey(const std::string& key)
    {
        m_backupKey = key;
    }

    inline std::unordered_map<std::string, std::unordered_set<Token>>& tokens()
    {
        return m_tokens;
    }

    bool isAlive(const types::Time& compareTo = 0L) const;

    void addToken(const std::string&, const Token& token);
    void removeToken(const std::string&, const std::string& token);
    bool isValidToken(const std::string&,
                      const std::string&,
                      const Registry*,
                      const types::Time& compareTo = 0L) const;

private:
    std::string m_id;
    types::Time m_dateCreated;
    unsigned int m_age;
    std::string m_rsaPublicKey;
    std::string m_key;
    int m_keySize;

    bool m_acknowledged;
    bool m_isKnown;

    std::unordered_map<std::string, std::unordered_set<Token>> m_tokens;

    // a backup key is previously set key with potentially different key size
    // see https://github.com/muflihun/residue/issues/75
    std::string m_backupKey;
};
}

namespace std {
template<> struct hash<residue::Client> {
public:
    std::size_t operator()(const residue::Client& c) const {
        return hash<std::string>{}(c.id());
    }
};
}

#endif /* Client_h */
