//
//  stats.cc
//  Residue
//
//  Copyright 2017-present Muflihun Labs
//
//  Author: @abumusamq
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#include "src/cli/stats.h"
#include "src/core/registry.h"

using namespace residue;

Stats::Stats(Registry* registry) :
    Command("stats",
            "Display stats for all the sessions (bytes received and sent)",
            "stats",
            registry)
{
}

void Stats::execute(std::vector<std::string>&&, std::ostringstream& result, bool) const
{
    result << "Recv: " << registry()->bytesReceived() << ", Sent: " << registry()->bytesSent();
}
