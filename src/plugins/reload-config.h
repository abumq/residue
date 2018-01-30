//
//  reload-config.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef ReloadConfig_h
#define ReloadConfig_h

#include "src/plugins/plugin.h"

namespace residue {

class Registry;

///
/// \brief Reload config plugin
///
class ReloadConfig : public Plugin
{
public:
    ReloadConfig(Registry* registry);

    virtual void execute(std::vector<std::string>&&, std::ostringstream&, bool) const override;

};
}

#endif /* ReloadConfig_h */