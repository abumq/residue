//
//  url.h
//  Residue
//
//  Copyright 2017-present @abumq (Majid Q.)
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

#ifndef Url_h
#define Url_h

#include <string>

namespace residue {

///
/// \brief URL class with various parts
///
class Url final {
public:
    static const std::string kProtocolEnd;
    static const char kPathSeparator;
    static const char kPortSeparator;

    explicit Url(const std::string& url);
    Url(const Url&);
    Url& operator=(Url);

    inline std::string protocol() const
    {
        return m_protocol;
    }

    inline std::string host() const
    {
        return m_host;
    }

    inline std::string path() const
    {
        return m_path;
    }

    inline std::string query() const
    {
        return m_query;
    }

    inline std::string port() const
    {
        return m_port;
    }

    inline bool isValid() const
    {
        return !m_protocol.empty() && !m_host.empty() && !m_port.empty();
    }

    bool isHttp() const;
private:
    std::string m_protocol;
    std::string m_host;
    std::string m_port;
    std::string m_path;
    std::string m_query;
};

}
#endif /* Utils_h */
