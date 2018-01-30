//
//  log-rotator.h
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

#ifndef LogRotator_h
#define LogRotator_h

#include <unordered_map>
#include <vector>
#include <tuple>
#include "src/tasks/task.h"

namespace residue {

class Registry;

///
/// \brief Log rotator task
///
class LogRotator final : public Task
{
public:
    ///
    /// \brief If log rotator is running and frequency is this threshold (in seconds) away,
    /// it will run it anyway
    ///
    static const unsigned long LENIENCY_THRESHOLD;

    using ArchiveItem = std::tuple<std::string, std::string, std::map<std::string, std::string>>;

    explicit LogRotator(Registry* registry,
                        unsigned int interval);
    void rotate(const std::string& loggerId);
    void archiveRotatedItems();
    std::string checkStatus(const std::string& loggerId);
protected:
    virtual void execute() override;
private:
    void archiveAndCompress(const std::string&,
                            const std::string&,
                            const std::map<std::string, std::string>&);

    std::vector<ArchiveItem> m_archiveItems;
    std::unordered_map<std::string, unsigned long> m_lastRotation;
};
}
#endif /* LogRotator_h */
