//
//  aes.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifdef RESIDUE_USE_MINE
#include "deps/mine/mine.h"
#else
#include "deps/ripe/Ripe.h"
#endif
#include "include/log.h"
#include "src/net/session.h"
#include "src/crypto/aes.h"

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
#ifdef RESIDUE_USE_MINE
    try {
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
