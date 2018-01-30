//
//  stats.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef Stats_h
#define Stats_h

#include "src/plugins/plugin.h"

namespace residue {

class Registry;

///
/// \brief Stats plugin
///
class Stats : public Plugin
{
public:
    Stats(Registry* registry);

    virtual void execute(std::vector<std::string>&&, std::ostringstream&, bool) const override;

};
}

#endif /* Stats_h */