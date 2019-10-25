//
//  utils-test.h
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

#ifndef UTILS_TEST_H
#define UTILS_TEST_H

#include <cstdio>

#include "test.h"

#include "utils/utils.h"

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

TEST(UtilsTest, BytesToHumanReadable)
{
    ASSERT_EQ(Utils::bytesToHumanReadable(999), "999B");
    ASSERT_EQ(Utils::bytesToHumanReadable(1025), "1.0kB");
    ASSERT_EQ(Utils::bytesToHumanReadable(1127), "1.1kB");
    ASSERT_EQ(Utils::bytesToHumanReadable(2048), "2kB");
    ASSERT_EQ(Utils::bytesToHumanReadable(205000), "200.2kB");
    ASSERT_EQ(Utils::bytesToHumanReadable(205950), "201.1kB");
    ASSERT_EQ(Utils::bytesToHumanReadable(20493322), "19.5MB");
    ASSERT_EQ(Utils::bytesToHumanReadable(static_cast<long>(1024.0f * 40000000.0f)), "38.1GB");
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

TEST(UtilsTest, ReplaceAll)
{
    std::string str("hello");
    ASSERT_EQ(Utils::replaceAll(str, "ll", "ww"), "hewwo");
    str = "this is 'test'";
    ASSERT_EQ(Utils::replaceAll(str, "'", "\\'"), "error Utils::replaceAll => <too many occurrences>. See <incr> parameter");
    str = "this is 'test'";
    ASSERT_EQ(Utils::replaceAll(str, "'", "\\'", 2), "this is \\'test\\'");
}

TEST(UtilsTest, IsJSON)
{
    static TestData<std::string, bool> Data = {
        { "da024686f7f2da49da6c98253b42fe1c:erezutlgudgbtwza:i3eclcagfnUbK1B==", false },
        { "da024686f7f2da49da6c98253b42fe1c:i3eclcagfnUbK1B==", false },
        { "erezutlgudgbtwza:i3eclcagfnUbK1B==", false },
        { "i3eclcagfnUbK1B==", false },
        { "{'json':true}", true },
        { "   { ' json':true}", true },
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
        { "123", "123", "246" },
        { "1", "123", "124" },
        { "123", "2", "125" },
        { "12", "123", "135" },
        { "123", "14", "137" },
        { "123456789", "987654321", "1111111110" },
        { "234324828942934", "258149112886176", "492473941829110" },
        { "234324828942934", "23824283943242", "258149112886176" },
        { "234324828942934258149112886176258149112886176", "234324828942934258149112886176258149112886176", "468649657885868516298225772352516298225772352" },
    };
    for (const auto& item : Data) {
        auto first = PARAM(0);
        auto second = PARAM(1);
        auto expected = PARAM(2);
        //std::cout << std::endl << std::endl;
        //TIMED_BLOCK(timer, std::string("Add Big Num\n    " + first + "\n +  " + second + "\n-------------------\n =  " + expected + "\n").c_str())
        {
            Utils::bigAdd(first, std::move(second));
        }
        //std::cout << std::endl;
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

TEST(UtilsTest, CreatePath)
{
    int r = system("rm -rf a_custom_directory");
    ASSERT_EQ(r, 0);
    ASSERT_FALSE(Utils::fileExists("a_custom_directory/is/a/path"));
    Utils::createPath("a_custom_directory/is/a/path");
    ASSERT_TRUE(Utils::createPath("a_custom_directory/is/a/path"));
    ASSERT_FALSE(Utils::fileExists("a_custom_directory/is/a/path_no"));
    ASSERT_TRUE(Utils::fileExists("a_custom_directory/is/a/path"));

    int r2 = system("rm -rf a_custom_directory2");
    ASSERT_EQ(r2, 0);
    ASSERT_FALSE(Utils::fileExists("a_custom_directory2/is/a/path/"));
    Utils::createPath("a_custom_directory2/is/a/path/");
    ASSERT_TRUE(Utils::createPath("a_custom_directory2/is/a/path/"));
    ASSERT_TRUE(Utils::fileExists("a_custom_directory2/is/a/path/"));
}

TEST(UtilsTest, TimeToTmTest)
{
    types::Time epoch = 1540988497; // Wed, 31/Oct/2018 23:21:37
    tm time = Utils::timeToTm(epoch);
    ASSERT_EQ(time.tm_sec, 37);
    ASSERT_EQ(time.tm_min, 21);
    ASSERT_EQ(time.tm_hour, 23);
    ASSERT_EQ(time.tm_mday, 31);
    ASSERT_EQ(time.tm_mon, 10 - 1);
    ASSERT_EQ(time.tm_year + 1900, 2018);
    ASSERT_STREQ(time.tm_zone, "AEDT");
}

#endif // UTILS_TEST_H
