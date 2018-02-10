//
//  command.h
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

#ifndef Command_h
#define Command_h

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
#include "logging/log.h"

namespace residue {

class Registry;

///
/// \brief Base command interface
///
class Command
{
public:
    Command(const std::string& name,
            const std::string& description,
            const std::string& help,
            Registry* registry);

    virtual ~Command() = default;

    inline std::string name() const
    {
        return m_name;
    }

    inline std::string description() const
    {
        return m_description;
    }

    inline std::string help() const
    {
        return m_help;
    }

    inline Registry* registry() const
    {
        return m_registry;
    }

    bool getConfirmation(const std::string& help) const;

    inline bool hasParam(const std::vector<std::string>& params, const std::string& param) const
    {
        return std::find(params.begin(), params.end(), param) != params.end();
    }

    ///
    /// \brief getParamValue Get param value e.g, --client-id blah will return blah
    ///
    std::string getParamValue(const std::vector<std::string>& params, const std::string& param) const;

    virtual void execute(std::vector<std::string>&&, std::ostringstream&, bool) const = 0;

private:
    std::string m_name;
    std::string m_description;
    std::string m_help;
    Registry* m_registry;
};
}

#endif /* Command_h */
