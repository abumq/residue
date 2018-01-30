//
//  session-details.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef SessionDetails_h
#define SessionDetails_h

#include "src/plugins/plugin.h"

namespace residue {

class Registry;

///
/// \brief SessionDetails plugin
///
class SessionDetails : public Plugin
{
public:
    SessionDetails(Registry* registry);

    virtual void execute(std::vector<std::string>&&, std::ostringstream&, bool) const override;

};
}

#endif /* SessionDetails_h */