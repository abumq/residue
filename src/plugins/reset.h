//
//  reset.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef Reset_h
#define Reset_h

#include "src/plugins/plugin.h"

namespace residue {

class Registry;

///
/// \brief Reset plugin
///
class Reset : public Plugin
{
public:
    Reset(Registry* registry);

    virtual void execute(std::vector<std::string>&&, std::ostringstream&, bool) const override;

};
}

#endif /* Reset_h */