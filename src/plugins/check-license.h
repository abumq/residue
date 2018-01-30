//
//  check-license.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef CheckLicense_h
#define CheckLicense_h

#include "src/plugins/plugin.h"

namespace residue {

class Registry;

///
/// \brief Check license plugin
///
class CheckLicense : public Plugin
{
public:
    CheckLicense(Registry* registry);

    virtual void execute(std::vector<std::string>&&, std::ostringstream&, bool) const override;

};
}

#endif /* CheckLicense_h */