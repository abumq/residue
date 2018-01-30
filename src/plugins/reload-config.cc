//
//  reload-config.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include "src/plugins/reload-config.h"
#include "src/core/registry.h"
#include "src/core/configuration.h"

using namespace residue;

ReloadConfig::ReloadConfig(Registry* registry) :
    Plugin("reload",
              "Reloads configuration",
              "reload",
              registry)
{
}

void ReloadConfig::execute(std::vector<std::string>&&, std::ostringstream& result, bool) const
{
    Configuration tmpConf(registry()->configuration()->configurationFile());
    if (tmpConf.isValid()) {
        result << "Reloading configurations...";
        registry()->configuration()->reload();
    } else {
        result << "FAILED to reload configuration. There are errors in configuration file" << std::endl << tmpConf.errors();
    }
}