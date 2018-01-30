//
//  update.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include "src/plugins/update.h"
#include "src/core/registry.h"
#include "src/tasks/auto-updater.h"

using namespace residue;

Update::Update(Registry* registry) :
    Plugin("update",
              "Check for updates and update the server.",
              "update [--check-only]",
              registry)
{
}

void Update::execute(std::vector<std::string>&& params, std::ostringstream& result, bool) const
{
    if (params.empty() || params.at(0) == "--check-only") {
        std::string newVer;
        if (registry()->autoUpdater()->check(&newVer)) {
            result << "You are running v" << RESIDUE_VERSION << ", new version is available: " << newVer << std::endl;
        } else {
            result << "You are running latest version v" << RESIDUE_VERSION << std::endl;
        }
    }
}