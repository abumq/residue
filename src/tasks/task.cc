//
//  task.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include <thread>
#include <chrono>
#include "include/log.h"
#include "src/tasks/task.h"
#include "src/logging/custom-logging.h"
#include "src/utils/utils.h"

using namespace residue;

Task::Task(const std::string& name,
           Registry* registry,
           unsigned int intervalInSeconds,
           bool roundOffToNearestHour) :
    m_name(name),
    m_registry(registry),
    m_interval(std::chrono::seconds(intervalInSeconds)),
    m_roundOff(roundOffToNearestHour),
    m_nextExecution(0UL),
    m_lastExecution(0UL),
    m_executing(false)
{
    m_nextExecution = Utils::now() + m_interval.count();
}

void Task::start()
{
    m_started = Utils::now();
    RVLOG(RV_INFO) << "Scheduled [" << m_name << "] to run every " << m_interval;
    while (true) {
        std::this_thread::sleep_for(m_interval);
        if (m_executing) {
            RLOG(WARNING) << "Task [" << m_name << "] already running. Skipping!";
            continue;
        }
        if (m_roundOff) {
            unsigned long secondsUntilNearestHour = 3600 - (Utils::now() % 3600);
            std::this_thread::sleep_for(std::chrono::seconds(secondsUntilNearestHour));
        }
        m_executing = true;
        m_lastExecution = Utils::now();
        RVLOG(RV_INFO) << "Executing task [" << m_name << "]";
        execute();
        RVLOG(RV_INFO) << "Finished task [" << m_name << "]";
        m_executing = false;
        m_nextExecution = Utils::now() + m_interval.count();
    }
}