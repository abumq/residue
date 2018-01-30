//
//  reset.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include "src/plugins/reset.h"
#include "src/core/registry.h"

using namespace residue;

Reset::Reset(Registry* registry) :
    Plugin("reset",
              "Resets the server to default state",
              "reset",
              registry)
{
}

void Reset::execute(std::vector<std::string>&&, std::ostringstream& result, bool ignoreConfirmation) const
{
    if (ignoreConfirmation || getConfirmation("Reset does following things to the server:\n"
                                              "1. Reloads configurations\n"
                                              "2. Disconnect all the clients (and remove corresponding tokens)\n"
                                              "3. Disconnect active sessions\n")) {
        registry()->reset();
        result << "Reset successful";
    }
}