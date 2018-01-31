//
//  license-checker-task.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
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