//
//  client-integrity-task.h
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

#ifndef ClientIntegrityTask_h
#define ClientIntegrityTask_h

#include <unordered_set>
#include <mutex>
#include "tasks/task.h"

namespace residue {

class Registry;

///
/// \brief Task to check integrity of the clients.
/// If invalid, this task will remove it from the registry
///
class ClientIntegrityTask final : public Task
{
public:
    ClientIntegrityTask(Registry* registry, unsigned int interval);

    ///
    /// \brief Perform forceful clean up for specified client
    ///
    void performCleanup(const std::string&);

    inline void pauseClient(const std::string& clientId)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_pausedClients.find(clientId) == m_pausedClients.end()) {
            m_pausedClients.insert(clientId);
        }
    }

    inline void resumeClient(const std::string& clientId)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto pos = m_pausedClients.find(clientId);
        if (pos != m_pausedClients.end()) {
            m_pausedClients.erase(pos);
        }
    }
protected:
    virtual void execute() override;
private:
    std::unordered_set<std::string> m_pausedClients;
    std::mutex m_mutex;
};
}
#endif /* ClientIntegrityTask_h */
