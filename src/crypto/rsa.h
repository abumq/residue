//
//  rsa.h
//  Residue
//
//  Copyright 2017-present Zuhd Web Services
//  https://zuhd.org
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

#ifndef RSA_h
#define RSA_h

#include <string>

#include "static-base.h"

#ifdef RESIDUE_USE_MINE
#include "mine/mine.h"
#include <cryptopp/integer.h>
#include <cryptopp/pem-com.h> // for readPem func
#endif

#ifdef RESIDUE_USE_MINE
namespace mine {
using BigInt = CryptoPP::Integer;

class Helper : public MathHelper<BigInt>
{
public:
    virtual byte bigIntegerToByte(const BigInt& b) const override
    {
        return static_cast<byte>(b.ConvertToLong());
    }

    virtual void divideBigInteger(const BigInt& divisor, const BigInt& divident,
                                        BigInt* quotient, BigInt* remainder) const override
    {
        BigInt::Divide(*remainder, *quotient, divisor, divident);
    }

    virtual std::string bigIntegerToHex(BigInt b) const override
    {
        std::stringstream ss;
        ss << std::hex << b;
        std::string h(ss.str());
        h.erase(h.end() - 1);
        return h;
    }

    virtual std::string bigIntegerToString(const BigInt& b) const override
    {
        std::stringstream ss;
        ss << b;
        std::string h(ss.str());
        h.erase(h.end() - 1);
        return h;
    }
};

class RSAManager : public GenericRSA<BigInt, Helper> {};
class PublicKey : public GenericPublicKey<BigInt, Helper>
{
public:
    void loadFromPem(const std::string& contents)
    {
        CryptoPP::RSA::PublicKey keyOut;
        {
            using namespace CryptoPP;
            StringSource source(contents, true);
            PEM_Load(source, keyOut);
        }

        init(keyOut.GetModulus(), static_cast<int>(keyOut.GetPublicExponent().ConvertToLong()));
        m_pem = contents;
    }

    inline std::string pem() const { return m_pem; }
private:
    std::string m_pem;
};
class PrivateKey : public GenericPrivateKey<BigInt, Helper>
{
public:
    void loadFromPem(const std::string& contents, const std::string& secret)
    {
        CryptoPP::RSA::PrivateKey keyOut;
        {
            using namespace CryptoPP;
            StringSource source(contents, true);
            if (secret.empty()) {
                PEM_Load(source, keyOut);
            } else {
                PEM_Load(source, keyOut, secret.data(), secret.size());
            }
        }
        init(keyOut.GetPrime1(), keyOut.GetPrime2(), static_cast<int>(keyOut.GetPublicExponent().ConvertToLong()));
        m_pem = contents;
    }

    inline std::string pem() const { return m_pem; }
private:
    std::string m_pem;
};
class KeyPair : public GenericKeyPair<BigInt, Helper>
{
    using GenericKeyPair::GenericKeyPair;
};
} // namespace mine
#endif

namespace residue {

///
/// \brief RSA crypto wrappers
///
class RSA final : StaticBase
{
public:

#ifdef RESIDUE_USE_MINE
    using PrivateKey = mine::PrivateKey;
    using PublicKey = mine::PublicKey;

    static PrivateKey loadPrivateKey(const std::string& pem, const std::string& secret) {
        mine::PrivateKey pk;
        pk.loadFromPem(pem, secret);
        return pk;
    }

    static PublicKey loadPublicKey(const std::string& pem) {
        mine::PublicKey pk;
        pk.loadFromPem(pem);
        return pk;
    }
#else
    using PrivateKey = std::string;
    using PublicKey = std::string;

    static PrivateKey loadPrivateKey(const std::string& pem, const std::string& secret = "") {
        (void) secret;
        return pem;
    }

    static PublicKey loadPublicKey(const std::string& pem) {
        return pem;
    }

#endif

    struct KeyPair {
        PrivateKey privateKey;
        PublicKey publicKey;
    };

    static std::string encrypt(const std::string& data, const PublicKey& publicKey);

    static std::string decrypt(std::string& data, const PrivateKey& privateKey, const std::string& secret = "");

    static std::string sign(const std::string& data, const PrivateKey& key, const std::string& secret = "");

    static bool verify(const std::string& data, const std::string& signHex, const PublicKey& key);

    static bool verifyKeyPair(const PrivateKey& privateKey, const PublicKey& publicKey, const std::string& secret = "");

};
}

#endif /* RSA_h */
