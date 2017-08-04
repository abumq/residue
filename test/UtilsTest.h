//
//  UtilsTest.h
//  Residue Tests
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef UTILS_TEST_H
#define UTILS_TEST_H

#include <cstdio>
#include "test.h"
#include "src/Utils.h"

using namespace residue;

static const char* kUtilsTestFile = "file.tmp.txt";

TEST(UtilsTest, IsAlphaNumeric)
{
    ASSERT_FALSE(Utils::isAlphaNumeric("this is alphanumeric")); // spaces
    ASSERT_FALSE(Utils::isAlphaNumeric("this-is-not-alphanumeric"));
    ASSERT_TRUE(Utils::isAlphaNumeric("this-is-not-alphanumeric", "-"));
    ASSERT_TRUE(Utils::isAlphaNumeric("this-is-not-alphanumeric123", "-"));
    ASSERT_TRUE(Utils::isAlphaNumeric("this-is-not_alphanumeric", "-_"));
    ASSERT_TRUE(Utils::isAlphaNumeric("this-is-not_alphanumeric123", "-_"));
}

TEST(UtilsTest, GenerateRandomKey)
{
    std::string s = Utils::generateRandomKey(32);
    ASSERT_EQ(32 * 2 /* hex */, s.size());
    LOG(INFO) << "Generated key: " << s;

    s = Utils::generateRandomKey(16);
    ASSERT_EQ(16 * 2 /* hex */, s.size());
    LOG(INFO) << "Generated key: " << s;

    s = Utils::generateRandomKey(24);
    ASSERT_EQ(24 * 2 /* hex */, s.size());
    LOG(INFO) << "Generated key: " << s;
}

TEST(UtilsTest, FileExists)
{
    std::fstream fs;
    fs.open(kUtilsTestFile, std::fstream::out);
    fs << "Existing file contents";
    fs.flush();
    fs.close();
    ASSERT_TRUE(Utils::fileExists(kUtilsTestFile));
    remove(kUtilsTestFile);
    ASSERT_FALSE(Utils::fileExists(kUtilsTestFile));
}

TEST(UtilsTest, IsSymmetricDataFormat)
{
    static TestData<std::string, bool> Data = {
        TestCase("da024686f7f2da49da6c98253b42fe1c:erezutlgudgbtwza:i3eclcagfnUbK1B==", true),
        TestCase("da024686f7f2da49da6c98253b42fe1c:i3eclcagfnUbK1B==", true),
        TestCase("erezutlgudgbtwza:i3eclcagfnUbK1B==", false),
        TestCase("i3eclcagfnUbK1B==", false),
    };
    for (const auto& item : Data) {
        auto first = PARAM(0);
        auto second = PARAM(1);
        ASSERT_EQ(Utils::isRipeDataFormat(first), second);
    }
}

TEST(UtilsTest, IsJSON)
{
    static TestData<std::string, bool> Data = {
        TestCase("da024686f7f2da49da6c98253b42fe1c:erezutlgudgbtwza:i3eclcagfnUbK1B==", false),
        TestCase("da024686f7f2da49da6c98253b42fe1c:i3eclcagfnUbK1B==", false),
        TestCase("erezutlgudgbtwza:i3eclcagfnUbK1B==", false),
        TestCase("i3eclcagfnUbK1B==", false),
        TestCase("{'json':true}", true),
        TestCase("   { ' json':true}", true),
    };
    for (const auto& item : Data) {
        auto first = PARAM(0);
        auto second = PARAM(1);
        ASSERT_EQ(Utils::isJSON(first), second);
    }
}

TEST(UtilsTest, BigAdd)
{
    static TestData<std::string, std::string, std::string> Data = {
        TestCase("123", "123", "246"),
        TestCase("1", "123", "124"),
        TestCase("123", "2", "125"),
        TestCase("12", "123", "135"),
        TestCase("123", "14", "137"),
        TestCase("123456789", "987654321", "1111111110"),
        TestCase("234324828942934", "258149112886176", "492473941829110"),
        TestCase("234324828942934", "23824283943242", "258149112886176"),
        TestCase("234324828942934258149112886176258149112886176", "234324828942934258149112886176258149112886176", "468649657885868516298225772352516298225772352"),
    };
    for (const auto& item : Data) {
        auto first = PARAM(0);
        auto second = PARAM(1);
        auto expected = PARAM(2);
        std::cout << std::endl << std::endl;
        TIMED_BLOCK(timer, std::string("Add Big Num\n    " + first + "\n +  " + second + "\n-------------------\n =  " + expected + "\n").c_str()) {
            Utils::bigAdd(first, std::move(second));
        }
        std::cout << std::endl;
        ASSERT_EQ(first, expected);
    }
    std::cout << std::endl;
}

TEST(UtilsTest, RandomAlphaNumeric)
{
    int total = 1000;
    float same = 0.0f;
    std::vector<std::string> list;
    for (int i = 0; i < total; ++i) {
        std::string s1 = Utils::generateRandomString(16, true);
        std::string s2 = Utils::generateRandomString(16, true);
        ASSERT_NE(s1, s2);
        ASSERT_EQ(s1.size(), 16);
        ASSERT_EQ(s2.size(), 16);
        if (std::find(list.begin(), list.end(), s1) != list.end() ||
            std::find(list.begin(), list.end(), s2) != list.end()) {
            same++;
        }
        list.push_back(s1);
        list.push_back(s2);
    }
    float totalGenerated = static_cast<float>(total) * 2.0f; // s1 and s2
    LOG_IF(same > 0, WARNING) << ((same / totalGenerated) * 100.0f) << "% similar strings generated (" << same << " / " << totalGenerated << ")";
    LOG_IF(same == 0, INFO) << "100% random!";
}

#endif // UTILS_TEST_H
