//
//  license-checker-task.cc
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

#include "include/log.h"
#include "src/tasks/license-checker-task.h"
#include "src/core/registry.h"
#include "src/core/configuration.h"

using namespace residue;

LicenseCheckerTask::LicenseCheckerTask(Registry* registry,
                       unsigned int interval) :
    Task("LicenseCheckerTask", registry, interval)
{
}

void LicenseCheckerTask::execute()
{
    if (!licenseManager.validate(m_registry->configuration()->license(), false)) {
        RLOG(ERROR) << "Your license has expired. You will not be able to restart your server with this license."
                    << "Please obtain a new license before restarting. We will not kill the server (unless it crashes unexpectedly)"
                    << std::endl << m_registry->configuration()->license()->formattedExpiry();
    }
}
