#include "log.h"

int main(int argc, const char** argv)
{
    const std::string clientConfigFile = "residue.conf";

    try {
        Residue::loadConfiguration(clientConfigFile);
        Residue::connect();
        Residue::enableCrashHandler();
    } catch (const ResidueException& e) {
        std::cout << "EXCEPTION: " << e.what() << std::endl;
        displayErrors();
        return 1;
    }

    // Start of application
    LOG(INFO) << "This is first log from sample app";

    Residue::disconnect();
    return 0;
}
