//
//  task.cc
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

#include <thread>
#include <chrono>
#include "src/logging/log.h"
#include "src/tasks/task.h"
#include "src/logging/custom-logging.h"
#include "src/utils/utils.h"

using namespace residue;

Task::Task(const std::string& name,
           Registry* registry,
           unsigned int intervalInSeconds) :
    m_name(name),
    m_registry(registry),
    m_interval(std::chrono::seconds(intervalInSeconds)),
    m_nextExecution(0UL),
    m_lastExecution(0UL),
    m_executing(false)
{
}

void Task::start()
{
    rescheduleFrom(Utils::now());
    RVLOG(RV_INFO) << "Scheduled [" << m_name << "] to run every " << m_interval
                   << "; next execution at ["
                   << formattedNextExecution() << "]";

    while (true) {
        DRVLOG(RV_DEBUG) << "Scheduled [" << m_name << "] waiting... [" << m_nextWait << "]";
        std::this_thread::sleep_for(m_nextWait);
        if (m_executing) {
            RLOG(WARNING) << "Task [" << m_name << "] already running, started ["
                          << formattedLastExecution() << "]. Skipping!";
            continue;
        }
        m_executing = true;
        m_lastExecution = Utils::now();
        RVLOG(RV_INFO) << "Executing task [" << m_name << "]";
        execute();
        RVLOG(RV_INFO) << "Finished task [" << m_name << "]";
        m_executing = false;
        rescheduleFrom(Utils::now());
        RVLOG(RV_DEBUG) << "Rescheduled task [" << m_name << "] at [" << formattedNextExecution() << "]";
    }
}

types::Time Task::calculateRoundOff(types::Time) const
{
    return 0;
}

void Task::rescheduleFrom(types::Time now)
{
    types::Time roundOff = calculateRoundOff(now);
    if (roundOff > 0) {
        m_nextExecution = now + roundOff;
        m_nextWait = std::chrono::seconds(roundOff);
    } else {
        m_nextExecution = now + m_interval.count();
        m_nextWait = m_interval;
    }
}
