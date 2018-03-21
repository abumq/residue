//
//  client.cc
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

#include "core/client.h"

#include <chrono>
#include <mutex>

#include "connect/connection-request.h"
#include "core/configuration.h"
#include "core/registry.h"
#include "crypto/aes.h"
#include "logging/log.h"
#include "utils/utils.h"

using namespace residue;

static std::mutex s_mutex;

Client::Client(const ConnectionRequest* request):
    m_id(request->clientId()),
    m_age(0),
    m_rsaPublicKey(request->rsaPublicKey()),
    m_keySize(request->keySize() / 8),
    m_acknowledged(false)
{
    m_key = AES::generateKey(request->keySize());
    resetDateCreated();
    m_token = AES::generateKey(128);
}

Client::~Client()
{
    DRVLOG(RV_TRACE) << "~Client " << m_id;
}

bool Client::isAlive(const types::Time& compareTo) const
{
    if (m_age == 0) {
        return true;
    }
    return m_dateCreated + m_age >= (compareTo == 0 ? Utils::now() : compareTo);
}
