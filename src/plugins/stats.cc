//
//  stats.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include "src/plugins/stats.h"
#include "src/core/registry.h"

using namespace residue;

Stats::Stats(Registry* registry) :
    Plugin("stats",
              "Display stats for all the sessions (bytes received and sent)",
              "stats",
              registry)
{
}

void Stats::execute(std::vector<std::string>&&, std::ostringstream& result, bool) const
{
    result << "Recv: " << registry()->bytesReceived() << ", Sent: " << registry()->bytesSent() << std::endl;
}