//
//  task.h
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

#ifndef Task_h
#define Task_h

#include <string>
#include <atomic>
#include <chrono>
#include "non-copyable.h"
#include "utils/utils.h"

namespace residue {

class Registry;

///
/// \brief Generic task
///
class Task : NonCopyable
{
public:
    Task(const std::string& name,
         Registry* registry,
         unsigned int intervalInSeconds);
    virtual ~Task() = default;

    void start();

    inline bool isExecuting() const
    {
        return m_executing;
    }

    inline types::Time nextExecution() const
    {
        return m_nextExecution;
    }

    inline types::Time lastExecution() const
    {
        return m_lastExecution;
    }

    inline types::Time intervalCount() const
    {
        return m_interval.count();
    }

    inline std::string name() const
    {
        return m_name;
    }

    inline std::string formattedNextExecution() const
    {
        return formattedExecution(m_nextExecution);
    }

    inline std::string formattedLastExecution() const
    {
        return formattedExecution(m_lastExecution);
    }

    inline void setLastExecution(types::Time time)
    {
        m_lastExecution = time;
    }

    virtual types::Time calculateRoundOff(types::Time now) const;

    void rescheduleFrom(types::Time now);

    bool kickOff(bool scheduled = false);

protected:
    std::string m_name;
    Registry* m_registry;
    std::chrono::seconds m_interval;
    types::Time m_nextExecution;
    std::chrono::seconds m_nextWait;
    types::Time m_lastExecution;
    std::atomic<bool> m_executing;

private:
    inline std::string formattedExecution(types::Time e) const
    {
        return Utils::formatTime(e, "%H:%m:%s on %a %d %b, %Y");
    }

    virtual void execute() = 0;
};
}
#endif /* Task_h */
