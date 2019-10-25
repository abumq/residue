//
//  reload-config.h
//  Residue
//
//  Copyright 2017-present Amrayn Web Services
//  https://amrayn.com
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

#ifndef ReloadConfig_h
#define ReloadConfig_h

#include "cli/command.h"

namespace residue {

class Registry;

///
/// \brief Reload config command
///
class ReloadConfig final : public Command
{
public:
    explicit ReloadConfig(Registry* registry);

    virtual void execute(std::vector<std::string>&&, std::ostringstream&, bool) const override;
private:
    void reloadServerConfig(std::ostringstream& result) const;
    void reloadLoggerConfig(const std::string& loggerId, std::ostringstream& result) const;

};
}

#endif /* ReloadConfig_h */
