//
//  UrlTest.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef URL_TEST_H
#define URL_TEST_H

#include "test.h"
#include "src/Url.h"

using namespace residue;

static TestData<std::string, std::string, std::string, std::string, std::string, std::string> URLTestData = {
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

TEST(UrlTest, TestUrls)
{
    for (const auto& item : URLTestData) {
        LOG(INFO) << "Testing " << PARAM(0);
        Url url(PARAM(0));
        ASSERT_EQ(PARAM(1), url.protocol());
        ASSERT_EQ(PARAM(2), url.host());
        ASSERT_EQ(PARAM(3), url.port());
        ASSERT_EQ(PARAM(4), url.path());
        ASSERT_EQ(PARAM(5), url.query());
        ASSERT_TRUE(url.isValid());
    }
}


#endif // URL_TEST_H
