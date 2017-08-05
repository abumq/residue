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

using namespace residue;

void displayUsage() {
    std::cout << "residue-license-manager [--validate <file> <signature>] [--issue <licensee> <licensee_signature> <validation_period> <issuing_authority>]" << std::endl;
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

    for (int i = 0; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg == "--validate" && i < argc) {
            licenseFile = argv[++i];
        } else if (arg == "--signature" && i < argc) {
            signature = argv[++i];
        } else if (arg == "--issue" && i < argc) {
        }
    }

    residue::LicenseManager licenseManager;
    if (!licenseFile.empty()) {
        std::ifstream stream(licenseFile);
        if (!stream.is_open()) {
            std::cerr << "Failed to open file " << licenseFile << std::endl;
        } else {

            std::string licenseKey = std::string((std::istreambuf_iterator<char>(stream)),
                                              (std::istreambuf_iterator<char>()));
            stream.close();
            residue::License license;
            if (!licenseManager.validate(licenseKey, license, true, signature)) {
                std::cout << "License is not valid";
            } else {
                std::cout << "Licensed to " << license.licensee() << std::endl;
                std::cout << "Subscription is active until " << license.formattedExpiry() << std::endl << std::endl;
            }
        }
    }
    return 0;
}
