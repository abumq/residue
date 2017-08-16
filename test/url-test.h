//
//  url-test.h
//  Residue Tests
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef URL_TEST_H
#define URL_TEST_H

#include "test.h"
#include "src/net/url.h"

using namespace residue;

static TestData<std::string, std::string, std::string, std::string, std::string, std::string> URLTestData = {
    TestCase("https://192.168.1.100:3322/p1?q=1", "https", "192.168.1.100", "3322", "/p1", "q=1"),
    TestCase("http://192.168.1.19:3000/p2?q2=2", "http", "192.168.1.19", "3000", "/p2", "q2=2"),
    TestCase("HTTP://LOCALHOST:3000/PATH?QUERY1=1&QUERY2=2", "HTTP", "LOCALHOST", "3000", "/PATH", "QUERY1=1&QUERY2=2"),
    TestCase("http://localhost:3000/path?query1=1&query2=2", "http", "localhost", "3000", "/path", "query1=1&query2=2"),
    TestCase("http://localhost", "http", "localhost", "80", "", ""),
    TestCase("http://localhost:3000", "http", "localhost", "3000", "", ""),
    TestCase("http://localhost:3000/", "http", "localhost", "3000", "/", ""),
    TestCase("localhost:3000/", "http", "localhost", "3000", "/", ""),
    TestCase("localhost", "http", "localhost", "80", "", ""),
    TestCase("http://localhost", "http", "localhost", "80", "", ""),
    TestCase("https://localhost", "https", "localhost", "443", "", ""),
    TestCase("localhost/", "http", "localhost", "80", "/", ""),
    TestCase("HTTP://LOCALHOST:3000/PATH?QUERY1=1&QUERY2=2", "HTTP", "LOCALHOST", "3000", "/PATH", "QUERY1=1&QUERY2=2"),

};

void testUrl(const Url& url, const std::tuple<std::string, std::string, std::string, std::string, std::string, std::string>& item)
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

    Url url1(std::get<0>(item1));
    Url url2(std::get<0>(item2));
    Url url3(std::get<0>(item1));

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
