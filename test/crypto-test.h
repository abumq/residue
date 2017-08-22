//
//  crypto-test.h
//  Residue Tests
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef CRYPTO_TEST_H
#define CRYPTO_TEST_H

#include <cstdio>
#include <algorithm>
#include "test.h"
#include "src/crypto/aes.h"

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


#endif // CRYPTO_TEST_H
