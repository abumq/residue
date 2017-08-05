//
//  License Manager tool for Residue
//
//  https://muflihun.github.io/residue
//  Copyright © 2017 Muflihun Labs
//

#include <cstring>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include "src/LicenseManager.h"
#include "src/License.h"

using namespace residue;

void displayUsage() {
    std::cout << "residue-license-manager [--validate <file> --signature <signature>] [--issue --licensee <licensee> --signature <licensee_signature> --period <validation_period> --authority <issuing_authority>]" << std::endl;
}

void displayVersion() {
    std::cout << "License Manager v" << RESIDUE_VERSION << std::endl;
}

int main(int argc, char* argv[])
{

    if (argc > 1 && strcmp(argv[1], "--version") == 0) {
        displayVersion();
        return 0;
    }

    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        displayUsage();
        return 0;
    }

    std::string licenseFile;
    std::string signature;
    std::string licensee;
    std::string authority = "default";
    unsigned int period;
    bool issue = false;
    bool validate = false;

    for (int i = 0; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg == "--validate" && i < argc) {
            licenseFile = argv[++i];
            validate = true;
        } else if (arg == "--signature" && i < argc) {
            signature = argv[++i];
        } else if (arg == "--issue" && i < argc) {
            issue = true;
        } else if (arg == "--licensee" && i < argc) {
            licensee = argv[++i];
        } else if (arg == "--period" && i < argc) {
            period = static_cast<unsigned int>(atoi(argv[++i]));
        } else if (arg == "--authority" && i < argc) {
            authority = argv[++i];
        }
    }

    residue::LicenseManager licenseManager;
    if (validate && !licenseFile.empty()) {
        std::ifstream stream(licenseFile);
        if (!stream.is_open()) {
            std::cerr << "Failed to open file " << licenseFile << std::endl;
        } else {

            std::string licenseKey = std::string((std::istreambuf_iterator<char>(stream)),
                                              (std::istreambuf_iterator<char>()));
            stream.close();
            residue::License license;
            if (!licenseManager.validate(licenseKey, &license, true, signature)) {
                std::cout << "License is not valid";
            } else {
                std::cout << "Licensed to " << license.licensee() << std::endl;
                std::cout << "Subscription is active until " << license.formattedExpiry() << std::endl << std::endl;
            }
        }
    } else if (issue) {
#ifdef LICENSE_MANAGER_CAN_ISSUE_LICENSE
        licenseManager.changeIssuingAuthority(authority);
        License license = licenseManager.generateNew(licensee, period, signature);
        std::cout << license.toString() << std::endl;
        std::cout << "Licensed to " << license.licensee() << std::endl;
        std::cout << "Subscription is active until " << license.formattedExpiry() << std::endl << std::endl;
#else
        std::cerr << "ERROR: Access denied. You cannot issue new licenses." << std::endl;
#endif // LICENSE_MANAGER_CAN_ISSUE_LICENSE
    } else {
        displayUsage();
    }
    return 0;
}
