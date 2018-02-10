//
//  crypto-test.h
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

#ifndef CRYPTO_TEST_H
#define CRYPTO_TEST_H

#include <cstdio>
#include <algorithm>
#include "test.h"
#include "crypto/aes.h"
#include "crypto/zlib.h"
#include "utils/utils.h"

using namespace residue;

TEST(CryptoTest_AES, GenerateRandomKey)
{

    const int TOTAL = 1000;

    auto testBySize = [&](unsigned int size) {

        std::vector<std::string> list;
        for (int i = 0; i < TOTAL; ++i) {
            std::string s = AES::generateKey(size);
            ASSERT_EQ((size / 8) * 2 /* hex */, s.size());
            list.push_back(s);
        }

        ASSERT_EQ(list.size(), TOTAL);

        // ensure randomness
        std::sort(list.begin(), list.end());
        std::vector<std::string>::iterator it = std::unique(list.begin(), list.end());

        ASSERT_EQ(it, list.end()); // if this fails the list contained non-random items!
    };

    testBySize(256);
    testBySize(192);
    testBySize(128);
}

TEST(CryptoTest_ZLib, CompressionDecompression)
{

    std::string text = Utils::generateRandomString(10000);
    LOG(INFO) << "Compressing " << text.size() << " bytes";
    std::string out = ZLib::compress(text);
    float savingPerc = static_cast<float>(out.size()) / static_cast<float>(text.size()) * 100.0f;
    LOG(INFO) << "Result: " << out.size() << " bytes, saving: " << (savingPerc) << "%";
    std::string decompressed = ZLib::decompress(out);
    LOG(INFO) << "Recovered " << decompressed.size() << " bytes";
    ASSERT_STREQ(text.c_str(), decompressed.c_str());
}


#endif // CRYPTO_TEST_H
