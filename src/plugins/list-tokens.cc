//
//  list-tokens.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include "src/plugins/list-tokens.h"
#include "src/core/registry.h"

using namespace residue;

ListTokens::ListTokens(Registry* registry) :
    Plugin("tokens",
              "List all the tokens and their respective status for client",
              "tokens --client-id <id>",
              registry)
{
}

void ListTokens::execute(std::vector<std::string>&& params, std::ostringstream& result, bool) const
{
    const std::string clientId = getParamValue(params, "--client-id");
    if (clientId.empty()) {
        result << "\nNo client ID provided" << std::endl;
        return;
    }
    for (auto& c : registry()->clients()) {
        if (c.second.id() == clientId) {
            for (auto& l: c.second.tokens()) {
                result << "Logger: " << l.first << std::endl;
                int i = 1;
                for (auto& t: l.second) {
                    result << (i++) << " > " << t.data() << " Age: " << t.age() << "s " << (t.isValid() ? "" : "(EXPIRED)") << std::endl;
                }
            }
        }
    }
}