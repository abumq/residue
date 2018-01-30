//
//  update.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef Update_h
#define Update_h

#include "src/plugins/plugin.h"

namespace residue {

class Registry;

///
/// \brief Update plugin
///
class Update : public Plugin
{
public:
    Update(Registry* registry);

    virtual void execute(std::vector<std::string>&&, std::ostringstream&, bool) const override;

};
}

#endif /* Update_h */