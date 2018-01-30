//
//  rotate.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef Rotate_h
#define Rotate_h

#include "src/plugins/plugin.h"

namespace residue {

class Registry;

///
/// \brief Rotate plugin
///
class Rotate : public Plugin
{
public:
    Rotate(Registry* registry);

    virtual void execute(std::vector<std::string>&&, std::ostringstream&, bool) const override;

};
}

#endif /* Rotate_h */