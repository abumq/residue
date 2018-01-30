//
//  client.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include <chrono>
#include <mutex>
#include "include/log.h"
#include "src/core/configuration.h"
#include "src/clients/client.h"
#include "src/connect/connection-request.h"
#include "src/core/registry.h"
#include "src/crypto/aes.h"
#include "src/utils/utils.h"

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
}

Client::~Client()
{
    DRVLOG(RV_TRACE) << "~Client " << m_id;
}

bool Client::isAlive(const unsigned long& compareTo) const
{
    if (m_age == 0) {
        return true;
    }
    return m_dateCreated + m_age >= (compareTo == 0 ? Utils::now() : compareTo);
}

bool Client::isValidToken(const std::string& loggerId,
                          const std::string& token,
                          const Registry* registry,
                          const unsigned long& compareTo) const
{
    DRVLOG(RV_DEBUG) << "Checking token " << token << " (client [" << m_id << "])";
    if (!registry->configuration()->hasFlag(Configuration::Flag::REQUIRES_TOKEN) || // ignore tokens
        (registry->configuration()->hasFlag(Configuration::Flag::ALLOW_UNKNOWN_LOGGERS) && !registry->configuration()->isKnownLogger(loggerId)) || // unknown loggers allowed and this logger is actually unknown
        (registry->configuration()->hasFlag(Configuration::Flag::ALLOW_DEFAULT_ACCESS_CODE) && registry->configuration()->accessCodes().find(loggerId) == registry->configuration()->accessCodes().end())) { // loggers without access codes are allowed and this logger actually does not have access codes
        return true;
    }
    std::lock_guard<std::mutex> lock_(s_mutex);
    const auto& iter = m_tokens.find(loggerId);
    if (iter == m_tokens.end()) {
        return false;
    }
    const auto& tokenIter = std::find(iter->second.begin(), iter->second.end(), token);
    if (tokenIter != iter->second.end()) {
        return tokenIter->isValid(compareTo);
    }
    return false;
}

void Client::addToken(const std::string& loggerId,
                      const Token& token)
{
    DRVLOG(RV_DEBUG) << "Adding token " << token.data() << " (client [" << m_id << "])";
    std::lock_guard<std::mutex> lock_(s_mutex);
    const auto& it = m_tokens.find(loggerId);
    if (it == m_tokens.end()) {
        std::unordered_set<Token> singleTokenSet = { token };
        m_tokens[loggerId] = singleTokenSet;
    } else {
        it->second.insert(token);
    }
}

void Client::removeToken(const std::string& loggerId,
                         const std::string& token)
{
    DRVLOG(RV_DEBUG) << "Removing token " << token << " (client [" << m_id << "])";
    std::lock_guard<std::mutex> lock_(s_mutex);
    const auto& iter = m_tokens.find(loggerId);
    if (iter != m_tokens.end()) {
        const auto& tokenIter = std::find(iter->second.begin(), iter->second.end(), token);
        if (tokenIter != iter->second.end()) {
            iter->second.erase(tokenIter);
            if (iter->second.empty()) {
                m_tokens.erase(iter);
            }
        }
    }
}