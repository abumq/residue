//
//  request-handler.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include "include/log.h"
#include "src/core/request-handler.h"
#include "src/core/request.h"
#include "src/crypto/aes.h"
#include "src/clients/client.h"

using namespace residue;

RequestHandler::RequestHandler(Registry* registry) :
    m_session(nullptr),
    m_registry(registry)
{

}

DecryptedRequest RequestHandler::decryptRequest(const std::string& requestStr,
                                                const Request::StatusCode defaultStatus,
                                                const std::string& key,
                                                bool ignoreClient)
{
    std::string requestInput(std::move(requestStr));
    std::size_t length = requestInput.size();
    std::size_t pos = requestInput.find_first_of(':');
    bool hasManualKey = !key.empty();
    Client* existingClient = nullptr;
    if (length > 33 && pos == 32) {

        std::string iv = requestInput.substr(0, pos);
        requestInput = requestInput.substr(pos + 1);
        pos = requestInput.find_first_of(':');
        if (!ignoreClient && pos == std::string::npos) {
            return std::make_tuple(nullptr, requestInput, Request::StatusCode::BAD_REQUEST, "Malformed request. No client ID");
        }
        std::string clientId = requestInput.substr(0, pos);
#if RESIDUE_DEBUG
        DRVLOG(RV_DEBUG) << "Client: " << clientId;
        DRVLOG(RV_CRAZY) << "IV: " << iv;
#endif
        if (!ignoreClient && (existingClient = m_registry->findClient(clientId)) == nullptr) {
            return std::make_tuple(nullptr, requestInput, Request::StatusCode::BAD_REQUEST, "Client not connected yet");
        }
        std::string requestBase64 = requestInput.substr(pos + 1);
#if RESIDUE_DEBUG
        DRVLOG(RV_CRAZY) << "Data (base64): " << requestBase64;
#endif
        try {
            requestInput = AES::decrypt(requestBase64, !hasManualKey ? existingClient->key() : key, iv);
#if RESIDUE_DEBUG
            DRVLOG(RV_CRAZY) << "Plain request: " << requestInput;
#endif

        } catch (const std::exception& e) {
            RLOG(ERROR) << "Exception thrown during decryption: " << e.what();
        }

        return std::make_tuple(existingClient, requestInput, Request::StatusCode::CONTINUE, "");
    }
    return std::make_tuple(nullptr, requestInput, defaultStatus, "");
}