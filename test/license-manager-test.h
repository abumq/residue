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

    const IssuingAuthority* authority0 = &(UnitTestLicenseKeys::LICENSE_ISSUING_AUTHORITIES.at(0));
    const IssuingAuthority* authority1 = &(UnitTestLicenseKeys::LICENSE_ISSUING_AUTHORITIES.at(1));

    LOG(INFO) << "Generating licenceWithSignature using " << authority0->id();
    License licenceWithSignature = licenseManager.issue("residue unit-test", 24U, authority0, "", "fasdf");
    LOG(INFO) << "Generating licenceWithoutSignature using " << authority0->id();
    License licenceWithoutSignature = licenseManager.issue("residue unit-test", 24U, authority0);

    LOG(INFO) << "Generating licenseFromOtherAuthority using " << authority1->id();
    License licenseFromOtherAuthority = licenseManager.issue("residue unit-test license", 24U, authority1);

    ASSERT_TRUE(licenseManager.validate(&licenceWithSignature, true, "fasdf"));
    ASSERT_TRUE(licenseManager.validate(&licenceWithoutSignature, true, "fasdf"));
    ASSERT_FALSE(licenseManager.validate(&licenceWithSignature, true, "wrong-sign"));
    ASSERT_FALSE(licenseManager.validate(&licenceWithSignature, false)); // need signature
    ASSERT_FALSE(licenseManager.validate(&licenceWithSignature, true)); // need signature
    ASSERT_TRUE(licenseManager.validate(&licenceWithoutSignature, false, "fasdf"));
    ASSERT_TRUE(licenseManager.validate(&licenceWithoutSignature, false, ""));

    ASSERT_EQ(licenceWithSignature.licensee(), "residue unit-test");
    ASSERT_EQ(licenceWithoutSignature.licensee(), "residue unit-test");
    ASSERT_EQ(licenceWithSignature.issuingAuthorityId(), "unittest-issuer-1");
    ASSERT_EQ(licenceWithoutSignature.issuingAuthorityId(), "unittest-issuer-1");
    ASSERT_EQ(licenseFromOtherAuthority.issuingAuthorityId(), "unittest-issuer-2");

    ASSERT_TRUE(licenseManager.validate(&licenseFromOtherAuthority, false, ""));
}

#ifndef RESIDUE_ON_CI
TEST(LicenseManagerTest, LicenseGenerationAndVerificationUsingSecureAuthority)
{
    const IssuingAuthority* authority2 = &(UnitTestLicenseKeys::LICENSE_ISSUING_AUTHORITIES.at(2));
    LicenseManagerForTest licenseManager;
    LOG(INFO) << "Generating licenseWithSecureAuthority using " << authority2->id();
    License licenseWithSecureAuthority = licenseManager.issue("residue unit-test license", 25U, authority2, "unit-test-issuer-secret");
    ASSERT_EQ(licenseWithSecureAuthority.issuingAuthorityId(), "unittest-issuer-3");

    ASSERT_TRUE(licenseManager.validate(&licenseWithSecureAuthority, false));
}
#endif

TEST(LicenseManagerTest, VerificationUsingSecureAuthority)
{
    const IssuingAuthority* authority2 = &(UnitTestLicenseKeys::LICENSE_ISSUING_AUTHORITIES.at(2));
    std::string validLicense = "eyJhdXRob3JpdHlfc2lnbmF0dXJlIjoiN0UyMDgyNjkwRkJEMzcwOTA2NkFFQzExOTJGMEFDOUY4NjI0RkI0ODYwMTNCMDhEMjVDMEMxQTI2MDZERThGODk0MENEREFFQUFFMkM2NTM5QTI3RTJCNTI3RTc4NjZCQUFBNUQ1ODg1RkJENTIzMEQzOTk3OEVBQzE3REY3NDZCNUFBNUJCM0RFN0VFNTU1QzgyMEE5QzFBRUNEQzZGMDQyNEUzRTgyQzBCQkNBMzhEMjVFQkYxNzNFNTIwNUM1MjVFQ0I5NkJCNTQ2OUI5REJDOUIxMTY1RjU0OEE2RTkzREMxNDk2Q0FDMTFBMDhFNTJERTY0REEwQzQzMDQzQiIsImV4cGlyeV9kYXRlIjoyMjkxMTY3NDgwLCJpc3N1ZV9kYXRlIjoxNTAyNzY3NDgwLCJpc3N1aW5nX2F1dGhvcml0eSI6InVuaXR0ZXN0LWlzc3Vlci0zIiwibGljZW5zZWUiOiJyZXNpZHVlIHVuaXQtdGVzdCBsaWNlbnNlIn0=";

    LicenseManagerForTest licenseManager;
    LOG(INFO) << "Verify using " << authority2->id();
    License license;
    license.load(validLicense);
    ASSERT_TRUE(licenseManager.validate(&license, false));
}

TEST(LicenseManagerTest, VerificationUsingSecureAuthorityAndSignature)
{
    const IssuingAuthority* authority2 = &(UnitTestLicenseKeys::LICENSE_ISSUING_AUTHORITIES.at(2));
    std::string validLicense = "eyJhdXRob3JpdHlfc2lnbmF0dXJlIjoiNzRCNkUxNkE1ODQyNzZCRkM2MUM3QjFGQUEzQkY3QkQyNDM2RDZGRUI0M0MzQjEzRUY5RUY5MTdDRDhBRThDRUY1QTM4ODEzMkRCMzYyQjUzMjk3ODQ5ODQ5QTJBNDQ2MUVCRTk2Nzg4QUI4NkNBOTM1NkVFRDExN0I4OTg3MDQzNDM5QjE2QzdEMEY2RjczMUQzMjE4QjIyMzlBQThDRkM3ODRFOUM3N0M3NTlBMzZDNjUyQTBDREUwN0U0RjM2OEMxNEJDREM0MEY3MjM1Nzc1NjJGNUM4MkMxMzJGM0FEMzRFNTgxMTMxQTIyQjhDM0JCQTg1NjU0RTk4Mjk4OSIsImV4cGlyeV9kYXRlIjoyMjkxMTY4NTExLCJpc3N1ZV9kYXRlIjoxNTAyNzY4NTExLCJpc3N1aW5nX2F1dGhvcml0eSI6InVuaXR0ZXN0LWlzc3Vlci0zIiwibGljZW5zZWUiOiJyZXNpZHVlIHVuaXQtdGVzdCBsaWNlbnNlIiwibGljZW5zZWVfc2lnbmF0dXJlIjoiMzQzMjM4NjQzNTYxMzYzNjYzMzUzMDYxNjUzNzM2NjE2NjM2Mzc2NDM1MzgzMDYzMzQ2MjY2MzE2MTY0MzIzMDNBNkE3MTQzNzM2NTZFNjg1MDJGNUE1ODM0MzA2OTM5MkY2RDQ2NUE0RTUzNTgzMjc0NzIyQjVBNjg1OTYxNkE0ODU4NkU1ODM0Nzg0Qjc2MkY3QTZBNzMzRDBEMEEwRDBBIn0=";

    LicenseManagerForTest licenseManager;
    LOG(INFO) << "Verify using " << authority2->id();
    License license;
    license.load(validLicense);
    ASSERT_TRUE(licenseManager.validate(&license, true, "residue-unittest-signature"));
    ASSERT_FALSE(licenseManager.validate(&license, true, "wrong-signature"));
    ASSERT_FALSE(licenseManager.validate(&license, false));
}

#endif // LICENSE_MANAGER_TEST_H
