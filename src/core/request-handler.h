//
//  request-handler.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef RequestHandler_h
#define RequestHandler_h

#include <string>
#include "src/non-copyable.h"
#include "src/core/configuration.h"
#include "src/core/registry.h"
#include "src/core/request.h"
#include "src/crypto/base16.h"
#include "src/crypto/base64.h"
#include "src/crypto/rsa.h"
#include "src/crypto/zlib.h"

namespace residue {

class Session;
class Client;
class AdminRequest;
class ConnectionRequest;

///
/// \brief Raw request tuple created for each session
///
using RawRequest = std::tuple<
    std::string, // data
    std::string, // ip
    unsigned long // date_received
>;

///
/// \brief Type of incomming request
///
enum class RawRequestType {
    AES,
    RSA,
    JSON,
    UNKNOWN
};

///
/// \brief Request tuple with decrypted data
///
using DecryptedRequest = std::tuple<
    Client*,
    std::string, // request data
    Request::StatusCode,
    std::string // error text (if any)
>;

class RequestHandler : NonCopyable
{
public:
    explicit RequestHandler(Registry*);
    virtual ~RequestHandler() = default;

    RawRequestType getRequestType(const RawRequest&);

    virtual void handle(RawRequest&&) = 0;

    DecryptedRequest decryptRequest(const std::string& requestStr,
                                    const Request::StatusCode defaultStatus = Request::StatusCode::BAD_REQUEST,
                                    const std::string& key = "",
                                    bool ignoreClient = false);

    inline Registry* registry() const
    {
        return m_registry;
    }

    inline Configuration* configuration() const
    {
        return m_registry->configuration();
    }

    inline void setSession(Session* session)
    {
        m_session = session;
    }
protected:

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
        std::string& requestStr = std::get<0>(rawRequest);
        std::string& ipAddress = std::get<1>(rawRequest);
        unsigned long& dateReceived = std::get<2>(rawRequest);
        handle(std::move(requestStr), std::move(ipAddress), std::move(dateReceived), request, defaultStatus, tryServerRSAKey, tryServerAESKey, decompress);
    }

    ///
    /// \brief Takes raw requests (string) and turns it in to Request
    /// \see Request
    ///
    template <typename T = Request>
    void handle(std::string&& requestStr,
                std::string&& ipAddr,
                unsigned long&& dateReceived,
                T* request,
                Request::StatusCode defaultStatus = Request::StatusCode::BAD_REQUEST,
                bool tryServerRSAKey = false,
                bool tryServerAESKey = false,
                bool decompress = false)
    {
        auto decompressIfNeeded = [&](std::string& plainRequestStr) {
            if (decompress) {
                try {
                    plainRequestStr = ZLib::decompress(Base64::decode(plainRequestStr));
                } catch (const std::exception& e) {
                    // Only do verbose log as some libraries may send inflated data
                    // so we do not want to fill log with this error unless server admin
                    // chooses to do so with '-v' option
                    DRVLOG(RV_ERROR) << "Failed to decompress the data: " << e.what();
                }
            }
        };
#if RESIDUE_DEBUG
        DRVLOG(RV_DEBUG) << "Raw request: " << requestStr;
#endif
        DecryptedRequest dr = decryptRequest(requestStr, defaultStatus);

        request->m_client = std::get<0>(dr);

        std::string plainRequestStr = std::get<1>(dr);
        request->m_statusCode = std::get<2>(dr);
        request->m_errorText = std::get<3>(dr);
        request->m_ipAddr = std::move(ipAddr);
        request->m_dateReceived = std::move(dateReceived);
        decompressIfNeeded(plainRequestStr);
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
#endif
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
                request->m_client = std::get<0>(dr);
                std::string plainRequestStr = std::get<1>(dr);
                request->m_statusCode = std::get<2>(dr);
                request->m_errorText = std::get<3>(dr);
                request->deserialize(std::move(plainRequestStr));
            } catch (const std::exception& e) {
                DRVLOG(RV_ERROR) << e.what();
                // continue... as it's invalid anyway
            }
        }

        if (result && !usedRsaKey && tryServerRSAKey && request->m_client == nullptr && !m_registry->configuration()->hasFlag(Configuration::ALLOW_PLAIN_CONNECTION)) {
            // This will only happen when we have plain request
            request->m_errorText = "Plain connections not allowed by the server";
            request->m_statusCode = Request::StatusCode::BAD_REQUEST;
        }
    }

    Session* m_session;
    Registry* m_registry;
};
}
#endif /* RequestHandler_h */