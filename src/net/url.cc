//
//  url.cc
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

#include <cctype>
#include <algorithm>
#include <functional>
#include "logging/log.h"
#include "net/url.h"
#include "utils/utils.h"

using namespace residue;

const std::string Url::kProtocolEnd = "://";
const char Url::kPathSeparator = '/';
const char Url::kPortSeparator = ':';

Url::Url(const std::string& url)
{
    auto protocolPos = std::search(url.begin(), url.end(), kProtocolEnd.begin(), kProtocolEnd.end());

    if (protocolPos == url.end()) {
        m_protocol = "http";
        protocolPos = url.begin();
    } else {
        // reserve from start to position
        m_protocol.reserve(std::distance(url.begin(), protocolPos));
        std::copy(url.begin(), protocolPos, back_inserter(m_protocol));
        std::advance(protocolPos, kProtocolEnd.length());
    }

    auto pathPos = find(protocolPos, url.end(), kPathSeparator);
    m_host.reserve(distance(protocolPos, pathPos));
    std::copy(protocolPos, pathPos, back_inserter(m_host));

    auto portPos = find(protocolPos, url.end(), kPortSeparator);
    if (portPos + 1 != url.end() && portPos + 1 < pathPos) {
        m_port.reserve(distance(protocolPos, portPos));
        std::copy(portPos + 1, pathPos, back_inserter(m_port));
        m_host.erase(m_host.find_first_of(':'));
    } else {
        m_port = m_protocol == "https" ? "443" : "80"; // We only need these two protocols, don't worry about others
    }

    auto queryPos = find(pathPos, url.end(), '?');
    m_path.assign(pathPos, queryPos);

    if (queryPos != url.end()) {
        ++queryPos;
    }
    m_query.assign(queryPos, url.end());
}

Url::Url(const Url& other):
    m_protocol(other.m_protocol),
    m_host(other.m_host),
    m_port(other.m_port),
    m_path(other.m_path),
    m_query(other.m_query)
{
}

Url& Url::operator=(Url other)
{
    std::swap(m_host, other.m_host);
    std::swap(m_port, other.m_port);
    std::swap(m_path, other.m_path);
    std::swap(m_protocol, other.m_protocol);
    std::swap(m_query, other.m_query);
    return *this;
}

bool Url::isHttp() const
{
    std::string pCopy(m_protocol);
    Utils::toLower(pCopy);
    return pCopy == "http";
}
