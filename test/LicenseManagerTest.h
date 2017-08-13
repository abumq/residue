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

    licenseManager.changeIssuingAuthority("short-term-issuer");

    LOG(INFO) << "Generating licenceWithSignature";
    License licenceWithSignature = licenseManager.generateNew("residue unit-test", 24U, "", "fasdf");
    LOG(INFO) << "Generating licenceWithoutSignature";
    License licenceWithoutSignature = licenseManager.generateNew("residue unit-test", 24U);

    licenseManager.changeIssuingAuthority("beta-issuer");
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
    ASSERT_EQ(licenceWithSignature.issuingAuthorityId(), "short-term-issuer");
    ASSERT_EQ(licenceWithoutSignature.issuingAuthorityId(), "short-term-issuer");
    ASSERT_EQ(licenseFromOtherAuthority.issuingAuthorityId(), "beta-issuer");

    ASSERT_TRUE(licenseManager.validate(licenseFromOtherAuthority, false, ""));
}

#endif // LICENSE_MANAGER_TEST_H
