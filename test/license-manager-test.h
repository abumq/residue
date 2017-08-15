//
//  license-manager-test.h
//  Residue Tests
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef LICENSE_MANAGER_TEST_H
#define LICENSE_MANAGER_TEST_H

#include "test.h"
#include "test/license-manager-for-test.h"
#include "src/licensing/license.h"

using namespace residue;

TEST(LicenseManagerTest, LicenseGenerationAndVerification)
{

    LicenseManagerForTest licenseManager;

    licenseManager.changeIssuingAuthority("unittest-issuer-1");

    LOG(INFO) << "Generating licenceWithSignature";
    License licenceWithSignature = licenseManager.generateNew("residue unit-test", 24U, "", "fasdf");
    LOG(INFO) << "Generating licenceWithoutSignature";
    License licenceWithoutSignature = licenseManager.generateNew("residue unit-test", 24U);

    licenseManager.changeIssuingAuthority("unittest-issuer-2");
    LOG(INFO) << "Generating licenseFromOtherAuthority using beta-issuer";
    License licenseFromOtherAuthority = licenseManager.generateNew("residue unit-test license", 24U);

    ASSERT_TRUE(licenseManager.validate(licenceWithSignature, true, "fasdf"));
    ASSERT_TRUE(licenseManager.validate(licenceWithoutSignature, true, "fasdf"));
    ASSERT_FALSE(licenseManager.validate(licenceWithSignature, true, "wrong-sign"));
    ASSERT_FALSE(licenseManager.validate(licenceWithSignature, false)); // need signature
    ASSERT_FALSE(licenseManager.validate(licenceWithSignature, true)); // need signature
    ASSERT_TRUE(licenseManager.validate(licenceWithoutSignature, false, "fasdf"));
    ASSERT_TRUE(licenseManager.validate(licenceWithoutSignature, false, ""));

    ASSERT_EQ(licenceWithSignature.licensee(), "residue unit-test");
    ASSERT_EQ(licenceWithoutSignature.licensee(), "residue unit-test");
    ASSERT_EQ(licenceWithSignature.issuingAuthorityId(), "unittest-issuer-1");
    ASSERT_EQ(licenceWithoutSignature.issuingAuthorityId(), "unittest-issuer-1");
    ASSERT_EQ(licenseFromOtherAuthority.issuingAuthorityId(), "unittest-issuer-2");

    ASSERT_TRUE(licenseManager.validate(licenseFromOtherAuthority, false, ""));
}

#endif // LICENSE_MANAGER_TEST_H
