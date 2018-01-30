//
//  session-details.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include "src/plugins/session-details.h"
#include "src/core/registry.h"
#include "src/utils/utils.h"

using namespace residue;

SessionDetails::SessionDetails(Registry* registry) :
    Plugin("sess",
              "Displays current session details e.g, active sessions etc",
              "sess [--stats]",
              registry)
{
}

void SessionDetails::execute(std::vector<std::string>&& params, std::ostringstream& result, bool) const
{
    result << "Active sessions: " << registry()->activeSessions().size() << std::endl;
    if (hasParam(params, "--stats")) {
        int i = 1;
        auto now = Utils::now();
        for (auto& session : registry()->activeSessions()) {
            result << (i++) << " > Recv: " << session.first->bytesReceived()
                   << ", Sent: " << session.first->bytesSent()
                   << ", Active for " << (now - session.second) << " s" << std::endl;
        }
    }
}