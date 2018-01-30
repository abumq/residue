//
//  clients.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef Clients_h
#define Clients_h

#include "src/plugins/plugin.h"

namespace residue {

class Registry;

///
/// \brief Clients plugin
///
class Clients : public Plugin
{
public:
    Clients(Registry* registry);

    virtual void execute(std::vector<std::string>&&, std::ostringstream&, bool) const override;

private:
    void list(std::ostringstream&) const;
};
}

#endif /* Clients_h */