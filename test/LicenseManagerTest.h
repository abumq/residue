//
//  UtilsTest.h
//  Residue Tests
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef LICENSE_MANAGER_TEST_H
#define LICENSE_MANAGER_TEST_H

#include "test.h"
#include "src/LicenseManager.h"

using namespace residue;

TEST(LicenseManagerText, IsAlphaNumeric)
{

    LicenseManager l;
    std::string licenceWithSignature = l.generateNew("muflihun labs", 25U, "fasdf");
    std::string licenceWithoutSignature = l.generateNew("muflihun labs", 25U);

    ASSERT_TRUE(std::get<0>(l.validate(licenceWithSignature, true, "fasdf")));
    ASSERT_TRUE(std::get<0>(l.validate(licenceWithoutSignature, true, "fasdf")));
    ASSERT_FALSE(std::get<0>(l.validate(licenceWithSignature, true, "wrong-sign")));
    ASSERT_FALSE(std::get<0>(l.validate(licenceWithSignature, false))); // need signature
    ASSERT_FALSE(std::get<0>(l.validate(licenceWithSignature, true))); // need signature
    ASSERT_TRUE(std::get<0>(l.validate(licenceWithoutSignature, false, "fasdf")));
    ASSERT_TRUE(std::get<0>(l.validate(licenceWithoutSignature, false, "")));

    ASSERT_EQ(std::get<1>(l.validate(licenceWithSignature, true, "fasdf")), "muflihun labs");
    ASSERT_EQ(std::get<1>(l.validate(licenceWithoutSignature, true, "fasdf")), "muflihun labs");
}

#endif // LICENSE_MANAGER_TEST_H
