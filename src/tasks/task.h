//
//  task.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef Task_h
#define Task_h

#include <string>
#include <atomic>
#include <chrono>
#include "src/non-copyable.h"

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
                  unsigned int intervalInSeconds,
                  bool roundOffToNearestHour = false);
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

protected:
    virtual void execute() = 0;

    std::string m_name;
    Registry* m_registry;
    unsigned long m_started;
    std::chrono::seconds m_interval;
    bool m_roundOff;
    unsigned long m_nextExecution;
    unsigned long m_lastExecution;
    std::atomic<bool> m_executing;
};
}
#endif /* Task_h */