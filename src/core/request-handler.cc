//
//  request-handler.cc
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

#include "logging/log.h"
#include "core/request-handler.h"
#include "core/request.h"
#include "crypto/aes.h"
#include "clients/client.h"

using namespace residue;

RequestHandler::RequestHandler(const std::string& name, Registry* registry) :
    m_name(name),
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
            return { nullptr, requestInput, Request::StatusCode::BAD_REQUEST, "Malformed request. No client ID" };
        }
        std::string clientId = requestInput.substr(0, pos);
#ifdef RESIDUE_DEBUG
        DRVLOG(RV_DEBUG) << "Client: " << clientId;
        DRVLOG(RV_CRAZY) << "IV: " << iv;
#endif
        if (!ignoreClient && (existingClient = m_registry->findClient(clientId)) == nullptr) {
            return { nullptr, requestInput, Request::StatusCode::BAD_REQUEST, "Client not connected yet" };
        }
        std::string requestBase64 = requestInput.substr(pos + 1);
#ifdef RESIDUE_DEBUG
        DRVLOG(RV_CRAZY) << "Data (base64): " << requestBase64;
#endif

#ifdef RESIDUE_DEBUG
        DRVLOG(RV_CRAZY) << "Plain request: " << requestInput;
#endif
        DecryptedResult decryptedResult;

        if (hasManualKey) {
#ifdef RESIDUE_DEV
            DRVLOG(RV_DEBUG) << "Decryption: Trying with manual key";
#endif
            decryptedResult = decryptWithKey(requestBase64, iv, clientId, key);
        } else {
#ifdef RESIDUE_DEV
            DRVLOG(RV_DEBUG) << "Decryption: Trying with current key";
#endif
            decryptedResult = decryptWithKey(requestBase64, iv, clientId, existingClient->key());
            if (!decryptedResult.successful && !existingClient->backupKey().empty()) {
                RVLOG(RV_DEBUG) << "Decryption: Trying with backup key";
                decryptedResult = decryptWithKey(requestBase64, iv, clientId, existingClient->backupKey());
            }
        }
        if (!decryptedResult.successful) {
            RVLOG(RV_ERROR) << "Exception thrown during decryption: " << decryptedResult.errorText;
        } else {
            requestInput = std::move(decryptedResult.result);
        }


        return { existingClient, requestInput, Request::StatusCode::OK, "" };
    }
    return { nullptr, requestInput, defaultStatus, "" };
}

DecryptedResult RequestHandler::decryptWithKey(const std::string& requestBase64,
                                               std::string& iv,
                                               const std::string& clientId,
                                               const std::string& key) const
{
 #ifdef RESIDUE_DEV
    DRVLOG(RV_CRAZY) << "Ripe command: echo " << iv << ":" << clientId << ":" << requestBase64
                     << " | ripe -d --aes --key " << key
                     << " --base64";
 #else
    RESIDUE_UNUSED(clientId);
 #endif // RESIDUE_DEV
    try {
        std::string dataCopy = requestBase64;
        std::string result = AES::decrypt(dataCopy, key, iv);
        return { true, std::move(result), "" };
    } catch (const std::exception& e) {
        return { false, "", e.what() };
    }
    return { false, "", "No error could be extracted" };
}
