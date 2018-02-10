//
//  url-test.h
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

#ifndef URL_TEST_H
#define URL_TEST_H

#include "test.h"
#include "net/url.h"

using namespace residue;

using URLTestCase = TestCase<std::string, std::string, std::string, std::string, std::string, std::string>;

static TestDataW<URLTestCase> URLTestData = {
    { "https://192.168.1.100:3322/p1?q=1", "https", "192.168.1.100", "3322", "/p1", "q=1" },
    { "http://192.168.1.19:3000/p2?q2=2", "http", "192.168.1.19", "3000", "/p2", "q2=2" },
    { "HTTP://LOCALHOST:3000/PATH?QUERY1=1&QUERY2=2", "HTTP", "LOCALHOST", "3000", "/PATH", "QUERY1=1&QUERY2=2" },
    { "http://localhost:3000/path?query1=1&query2=2", "http", "localhost", "3000", "/path", "query1=1&query2=2" },
    { "http://localhost", "http", "localhost", "80", "", "" },
    { "http://localhost:3000", "http", "localhost", "3000", "", "" },
    { "http://localhost:3000/", "http", "localhost", "3000", "/", "" },
    { "localhost:3000/", "http", "localhost", "3000", "/", "" },
    { "localhost", "http", "localhost", "80", "", "" },
    { "http://localhost", "http", "localhost", "80", "", "" },
    { "https://localhost", "https", "localhost", "443", "", "" },
    { "localhost/", "http", "localhost", "80", "/", "" },
    { "HTTP://LOCALHOST:3000/PATH?QUERY1=1&QUERY2=2", "HTTP", "LOCALHOST", "3000", "/PATH", "QUERY1=1&QUERY2=2"},

};

void testUrl(const Url& url, const URLTestCase& item)
{
    ASSERT_EQ(url.protocol(), PARAM(1));
    ASSERT_EQ(url.host(), PARAM(2));
    ASSERT_EQ(url.port(), PARAM(3));
    ASSERT_EQ(url.path(), PARAM(4));
    ASSERT_EQ(url.query(), PARAM(5));
    ASSERT_TRUE(url.isValid());
}

TEST(UrlTest, TestUrls)
{
    for (const auto& item : URLTestData) {
        LOG(INFO) << "Testing " << PARAM(0);
        Url url(PARAM(0));
        testUrl(url, item);
    }
}

TEST(UrlTest, TestUrlsCopy)
{
    for (const auto& item : URLTestData) {
        LOG(INFO) << "Testing " << PARAM(0);
        Url url(PARAM(0));
        Url url2(url);
        testUrl(url2, item);
    }
}

TEST(UrlTest, TestUrlsAssignment)
{
    const auto& item1 = URLTestData.at(0);
    const auto& item2 = URLTestData.at(1);

    Url url1(item1.get<0>());
    Url url2(item2.get<0>());
    Url url3(item1.get<0>());

    testUrl(url1, item1);
    testUrl(url2, item2);

    url1 = url2;

    testUrl(url2, item2);
    testUrl(url1, item2);

    url2 = url3;

    testUrl(url2, item1);
    testUrl(url1, item2);
}


#endif // URL_TEST_H
