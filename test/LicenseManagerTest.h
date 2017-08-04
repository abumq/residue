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

    ASSERT_TRUE(l.validate(licenceWithSignature, true, "fasdf"));
    ASSERT_TRUE(l.validate(licenceWithoutSignature, true, "fasdf"));
    ASSERT_FALSE(l.validate(licenceWithSignature, true, "wrong-sign"));
    ASSERT_FALSE(l.validate(licenceWithSignature, false)); // need signature
    ASSERT_FALSE(l.validate(licenceWithSignature, true)); // need signature
    ASSERT_TRUE(l.validate(licenceWithoutSignature, false, "fasdf"));
    ASSERT_TRUE(l.validate(licenceWithoutSignature, false, ""));
}

#endif // LICENSE_MANAGER_TEST_H
