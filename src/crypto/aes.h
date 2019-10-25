//
//  aes.h
//  Residue
//
//  Copyright 2017-present Amrayn Web Services
//  https://amrayn.com
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

#ifndef AES_h
#define AES_h

#include <string>
#include "static-base.h"

namespace residue {

///
/// \brief AES (CBC) crypto wrappers
///
class AES final : StaticBase
{
public:

    ///
    /// \brief Decrypts AES-CBC
    /// \param raw Base64 encoded raw data (provided by ref)
    /// \param initVector Initialization vector (hex) for CBC. This is provided by reference in case it needs
    /// to be "normalized" i.e from AE2A => AE 2A for reability
    ///
    static std::string decrypt(std::string& raw, const std::string& hexKey, std::string& initVector);

    ///
    /// \brief This returns in following format:
    /// <pre>
    /// [iv]:[base64-encoded-cipher]
    /// </pre>
    ///
    static std::string encrypt(const std::string& raw, const std::string& hexKey, const std::string& initVector = "");

    ///
    /// \brief Generates a random key in bits.
    ///
    static std::string generateKey(unsigned int bits);
};
}

#endif /* AES_h */
