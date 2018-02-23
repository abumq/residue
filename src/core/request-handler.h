//
//  request-handler.h
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

#ifndef RequestHandler_h
#define RequestHandler_h

#include <memory>
#include <string>
#include "non-copyable.h"
#include "core/configuration.h"
#include "core/registry.h"
#include "core/request.h"
#include "crypto/base16.h"
#include "crypto/base64.h"
#include "crypto/rsa.h"
#include "crypto/zlib.h"

namespace residue {

class Session;
class Client;
class AdminRequest;
class ConnectionRequest;

///
/// \brief Raw request structure created for each session
///
struct RawRequest
{
    std::string data;
    std::string ip;
    types::Time dateReceived;
    std::shared_ptr<Session> session;
};

///
/// \brief Request structure with decrypted data
///
struct DecryptedRequest
{
    Client* client;
    std::string plainRequestStr;
    Request::StatusCode statusCode;
    std::string errorText;
};

///
/// \brief This is low level decryption result with various keys
///
struct DecryptedResult
{
    bool successful;
    std::string result;
    std::string errorText;
};

///
/// \brief Type of incomming request
///
enum class RawRequestType
{
    AES,
    RSA,
    JSON,
    UNKNOWN
};

class RequestHandler : NonCopyable
{
public:
    RequestHandler(const std::string& name, Registry*);
    virtual ~RequestHandler() = default;

    RawRequestType getRequestType(const RawRequest&);

    virtual void handle(RawRequest&&) = 0;

    DecryptedRequest decryptRequest(const std::string& requestStr,
                                    const Request::StatusCode defaultStatus = Request::StatusCode::BAD_REQUEST,
                                    const std::string& key = "",
                                    bool ignoreClient = false);

    inline const std::string& name() const
    {
        return m_name;
    }

    inline Registry* registry() const
    {
        return m_registry;
    }

    inline Configuration* configuration() const
    {
        return m_registry->configuration();
    }
protected:
    std::string m_name;
    Registry* m_registry;

    ///
    /// \brief Takes raw requests and turns it in to Request
    /// \see handle(std::string&& requestStr, T* request)
    ///
    template <typename T = Request>
    inline void handle(RawRequest&& rawRequest,
                       T* request,
                       Request::StatusCode defaultStatus = Request::StatusCode::BAD_REQUEST,
                       bool tryServerRSAKey = false,
                       bool tryServerAESKey = false,
                       bool decompress = false)
    {
        handle(std::move(rawRequest.data), std::move(rawRequest.ip), std::move(rawRequest.dateReceived),
               request, defaultStatus, tryServerRSAKey, tryServerAESKey, decompress);
    }

    ///
    /// \brief Takes raw requests (string) and turns it in to Request
    /// \see Request
    ///
    template <typename T = Request>
    void handle(std::string&& requestStr,
                std::string&& ipAddr,
                types::Time&& dateReceived,
                T* request,
                Request::StatusCode defaultStatus = Request::StatusCode::BAD_REQUEST,
                bool tryServerRSAKey = false,
                bool tryServerAESKey = false,
                bool decompress = false)
    {
#ifdef RESIDUE_DEBUG
        DRVLOG(RV_DEBUG) << "Raw request: " << requestStr;
#endif
        DecryptedRequest dr = decryptRequest(requestStr, defaultStatus);
#ifdef RESIDUE_DEV
        DRVLOG(RV_TRACE) << "Decryption finished (b64): " << dr.plainRequestStr;
#endif
        request->m_client = dr.client;

        std::string plainRequestStr = dr.plainRequestStr;
        request->m_statusCode = dr.statusCode;
        request->m_errorText = dr.errorText;
        request->m_ipAddr = std::move(ipAddr);
        request->m_dateReceived = std::move(dateReceived);
        if (decompress) {
#ifdef RESIDUE_DEV
            DRVLOG(RV_TRACE) << "Decompressing: " << plainRequestStr;
#endif
            try {
                plainRequestStr = ZLib::decompress(Base64::decode(plainRequestStr));
            }  catch (const std::exception& e) {
                // Only do verbose log as some libraries may send inflated data
                // so we do not want to fill log with this error unless server admin
                // chooses to do so with '-v' option
                DRVLOG(RV_ERROR) << "Failed to decompress the data: " << e.what();
            }
#ifdef RESIDUE_DEV
            DRVLOG(RV_TRACE) << "Decompression finished (raw): " << plainRequestStr;
#endif
        }
        bool result = request->deserialize(std::move(plainRequestStr));
        bool usedRsaKey = false;
        if (!result && tryServerRSAKey && !m_registry->configuration()->serverRSAKey().privateKey.empty()) {
            DRVLOG(RV_INFO) << "Trying with server exchange key...";
            // Try with server RSA key data
            try {
 #ifdef RESIDUE_USE_MINE
                std::string requestStrNormal = Base16::encode(Base64::decode(requestStr));
                std::string decryptedReq = RSA::decrypt(requestStrNormal,
                                                        m_registry->configuration()->serverRSAKey().privateKey);
 #else
                std::string decryptedReq = RSA::decrypt(requestStr,
                                                        m_registry->configuration()->serverRSAKey().privateKey,
                                                        m_registry->configuration()->serverRSASecret());
 #endif // RESIDUE_USE_MINE
                result = request->deserialize(std::move(decryptedReq));
                usedRsaKey = result;
            } catch (const std::exception& e) {
                DRVLOG(RV_ERROR) << e.what();
                // continue... as it's invalid anyway
            }
        }

        if (!result && tryServerAESKey && !m_registry->configuration()->serverKey().empty()) {
            DRVLOG(RV_INFO) << "Trying with server master key...";
            // Try with server AES key
            try {
                dr = decryptRequest(requestStr, defaultStatus, m_registry->configuration()->serverKey(), true);
                request->m_client = dr.client;
                std::string plainRequestStr = dr.plainRequestStr;
                request->m_statusCode = dr.statusCode;
                request->m_errorText = dr.errorText;
                request->deserialize(std::move(plainRequestStr));
            } catch (const std::exception& e) {
                DRVLOG(RV_ERROR) << e.what();
                // continue... as it's invalid anyway
            }
        }

        if (result && !usedRsaKey && tryServerRSAKey && request->m_client == nullptr
                && !m_registry->configuration()->hasFlag(Configuration::ALLOW_INSECURE_CONNECTION)) {
            // This will only happen when we have plain request
            request->m_errorText = "Plain connections not allowed by the server";
            request->m_statusCode = Request::StatusCode::BAD_REQUEST;
        }
    }

    DecryptedResult decryptWithKey(const std::string& requestBase64,
                                   std::string &iv,
                                   const std::string& clientId,
                                   const std::string& key) const;
};
}
#endif /* RequestHandler_h */
