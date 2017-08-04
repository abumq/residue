//
//  LicenseManagerTest.h
//  Residue Tests
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef LICENSE_MANAGER_TEST_H
#define LICENSE_MANAGER_TEST_H

#include "test.h"
#include "src/LicenseManager.h"
#include "src/License.h"

using namespace residue;

TEST(LicenseManagerTest, LicenseGenerationAndVerification)
{

    LicenseManager licenseManager;
    License licenceWithSignature = licenseManager.generateNew("muflihun labs", 25U, "fasdf");
    License licenceWithoutSignature = licenseManager.generateNew("muflihun labs", 25U);

    ASSERT_TRUE(licenseManager.validate(licenceWithSignature, true, "fasdf"));
    ASSERT_TRUE(licenseManager.validate(licenceWithoutSignature, true, "fasdf"));
    ASSERT_FALSE(licenseManager.validate(licenceWithSignature, true, "wrong-sign"));
    ASSERT_FALSE(licenseManager.validate(licenceWithSignature, false)); // need signature
    ASSERT_FALSE(licenseManager.validate(licenceWithSignature, true)); // need signature
    ASSERT_TRUE(licenseManager.validate(licenceWithoutSignature, false, "fasdf"));
    ASSERT_TRUE(licenseManager.validate(licenceWithoutSignature, false, ""));

    ASSERT_EQ(licenceWithSignature.licensee(), "muflihun labs");
    ASSERT_EQ(licenceWithoutSignature.licensee(), "muflihun labs");
}

#endif // LICENSE_MANAGER_TEST_H
