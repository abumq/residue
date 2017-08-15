//
//  license-manager-for-test.h
//  Residue Tests
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef LICENSE_MANAGER_FOR_TEST_H
#define LICENSE_MANAGER_FOR_TEST_H

#include "test.h"
#include "src/licensing/license-manager.h"
#include "src/licensing/license.h"

using namespace residue;

class LicenseManagerForTest : public LicenseManager {
public:

    static const std::vector<IssuingAuthority> LICENSE_ISSUING_AUTHORITIESf;

};

const std::vector<IssuingAuthority> LicenseManagerForTest::LICENSE_ISSUING_AUTHORITIESf = {

};


#endif // LICENSE_MANAGER_FOR_TEST_H
