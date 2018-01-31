//
//  license-checker-task.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef LicenseCheckerTask_h
#define LicenseCheckerTask_h

#include "src/tasks/task.h"
#include "src/licensing/license-manager.h"

namespace residue {

class Registry;

///
/// \brief License checker task
///
class LicenseCheckerTask final : public Task
{
public:
    explicit LicenseCheckerTask(Registry* registry,
                        unsigned int interval);
protected:
    virtual void execute() override;
    LicenseManager licenseManager;
};
}
#endif /* LicenseCheckerTask_h */