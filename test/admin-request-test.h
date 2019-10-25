//
//  admin-request-test.h
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

#ifndef ADMIN_REQUEST_TEST_H
#define ADMIN_REQUEST_TEST_H

#include "test.h"

#include <cstdio>

#include <vector>

#include "utils/utils.h"
#include "admin/admin-request.h"

using namespace residue;

TEST(AdminRequestTest, DeserializeTest)
{
    std::stringstream ss;
    ss << std::string(R"(
                      {"type": 8,
                      }
                      )");
    AdminRequest r(nullptr);
    r.setDateReceived(Utils::now());
    r.deserialize(ss.str());
    ASSERT_FALSE(r.isValid());

    ss.str("");

    ss << std::string(R"(
                      {"type": 8,
                      "_t":)") << Utils::now() << "}";

    RLOG(INFO) << "Admin req: " << ss.str();

    r.setDateReceived(Utils::now() + 1);
    r.deserialize(ss.str());
    ASSERT_TRUE(r.isValid());
}


#endif // ADMIN_REQUEST_TEST_H
