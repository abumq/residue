//
//  aes.cc
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

#include "crypto/aes.h"

#include "logging/log.h"
#include "net/session.h"

#ifdef RESIDUE_USE_MINE
#   include "mine/mine.h"
#   include "ripe/Ripe.h" // Mine encrypt is broken :(
#else
#   include "ripe/Ripe.h"
#endif

using namespace residue;

std::string AES::decrypt(std::string& raw, const std::string& key, std::string& iv)
{

#ifdef RESIDUE_USE_MINE
    try {
        mine::AES aes;
        return aes.decrypt(raw, key, iv, mine::MineCommon::Encoding::Base64, mine::MineCommon::Encoding::Raw);
    } catch (const std::exception& e) {
        // this can happen during initialization (static initialization)
        RLOG(ERROR) << "Failed to decrypt AES " << raw << ", e=" << e.what();
    }
    return "ERROR AES DECRYPT";
#else
    return Ripe::decryptAES(raw, key, iv, true);
#endif
}

std::string AES::encrypt(const std::string& plain, const std::string& key, const std::string& iv)
{
#ifdef RESIDUE_USE_MINE_DO_NOT_USE_BROKEN // see https://github.com/muflihun/mine/issues/11
    try {
        RLOG(INFO) << "Encrypting... " << plain;
        std::string ivCopy(iv);
        mine::AES aes;
        std::string ciphertext = aes.encrypt(plain, key, ivCopy, mine::MineCommon::Encoding::Raw, mine::MineCommon::Encoding::Base64);
        return ivCopy + ":" + ciphertext + Session::PACKET_DELIMITER;
    } catch (const std::exception& e) {
        // this can happen during initialization (static initialization)
        RLOG(ERROR) << "Failed to encrypt AES " << plain << ", e=" << e.what();
    }
    return "ERROR AES ENCRYPT";
#else
    std::string plainCopy(plain);
    return Ripe::encryptAES(plainCopy, key, "", "", iv);
#endif
}

std::string AES::generateKey(unsigned int bits)
{
#ifdef RESIDUE_USE_MINE
    return mine::AES::generateRandomKey(bits);
#else
    return Ripe::generateNewKey(bits / 8);
#endif
}
