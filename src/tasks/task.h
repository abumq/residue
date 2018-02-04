//
//  task.h
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

#ifndef Task_h
#define Task_h

#include <string>
#include <atomic>
#include <chrono>
#include "src/non-copyable.h"
#include "src/utils/utils.h"

namespace residue {

class Registry;

///
/// \brief Generic task
///
class Task : NonCopyable
{
public:
    explicit Task(const std::string& name,
                  Registry* registry,
                  unsigned int intervalInSeconds);
    virtual ~Task() = default;

    void start();

    inline bool isExecuting() const
    {
        return m_executing;
    }

    inline unsigned long nextExecution() const
    {
        return m_nextExecution;
    }

    inline unsigned long lastExecution() const
    {
        return m_lastExecution;
    }

    inline unsigned long intervalCount() const
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

    virtual unsigned long calculateRoundOff(unsigned long now = Utils::now()) const;

    void rescheduleFromNow();

protected:
    std::string m_name;
    Registry* m_registry;
    std::chrono::seconds m_interval;
    unsigned long m_nextExecution;
    std::chrono::seconds m_nextWait;
    unsigned long m_lastExecution;
    std::atomic<bool> m_executing;

private:
    inline std::string formattedExecution(unsigned long e) const
    {
        return Utils::formatTime(e, "%H:%m:%s on %a %d %b, %Y");
    }
    virtual void execute(unsigned long now = Utils::now()) = 0;
};
}
#endif /* Task_h */
