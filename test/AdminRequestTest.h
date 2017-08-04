//
//  AdminRequestTest.h
//  Residue Tests
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef ADMIN_REQUEST_TEST_H
#define ADMIN_REQUEST_TEST_H

#include <cstdio>
#include <vector>
#include "test.h"
#include "src/Utils.h"
#include "src/AdminRequest.h"

using namespace residue;

TEST(AdminRequestTest, DeserializeTest)
{
    std::stringstream ss;
    ss << std::string(R"(
                      {"type": 2,
                      "client_id": "blah",
                      "rsa_public_key": "blah"
                      }
                      )");
    AdminRequest r(nullptr);
    r.setDateReceived(Utils::now());
    r.deserialize(ss.str());
    ASSERT_FALSE(r.isValid());

    ss.str("");

    ss << std::string(R"(
                      {"type": 2,
                      "client_id": "blah",
                      "rsa_public_key": "blah",
                      "_t":999}
                      )");

    r.setDateReceived(1000);
    r.deserialize(ss.str());
    ASSERT_TRUE(r.isValid());
}


#endif // ADMIN_REQUEST_TEST_H
