//
//  license-checker-task.h
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
