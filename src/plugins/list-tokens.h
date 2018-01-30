//
//  list-tokens.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef ListTokens_h
#define ListTokens_h

#include "src/plugins/plugin.h"

namespace residue {

class Registry;

///
/// \brief ListTokens plugin
///
class ListTokens : public Plugin
{
public:
    ListTokens(Registry* registry);

    virtual void execute(std::vector<std::string>&&, std::ostringstream&, bool) const override;

};
}

#endif /* ListTokens_h */