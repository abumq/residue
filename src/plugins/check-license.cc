//
//  check-license.cc
//  Residue
//
//  Copyright 2017-present Muflihun Labs
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

#include "src/plugins/check-license.h"
#include "src/core/registry.h"
#include "src/core/configuration.h"

using namespace residue;

CheckLicense::CheckLicense(Registry* registry) :
    Plugin("license",
              "Displays current license status and other information",
              "license",
              registry)
{
}

void CheckLicense::execute(std::vector<std::string>&&, std::ostringstream&, bool) const
{
    /*LicenseManager licenseManager;
    if (licenseManager.validate(registry()->configuration()->license(), true, registry()->configuration()->licenseeSignature())) {
        result << "Licensed to " << registry()->configuration()->license()->licensee() << std::endl;
        result << "Subscription is active until " << registry()->configuration()->license()->formattedExpiry() << std::endl << std::endl;
    } else {
        result << "Your residue server license is no longer valid. Expiry: "
               << registry()->configuration()->license()->formattedExpiry() << std::endl;
        result << "Please buy a new subscription before restarting the server." << std::endl << std::endl;
    }*/
}
