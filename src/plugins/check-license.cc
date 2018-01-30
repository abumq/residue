//
//  check-license.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
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
