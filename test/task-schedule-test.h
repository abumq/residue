#ifndef TASK_SCHEDULE_TEST_H
#define TASK_SCHEDULE_TEST_H

#include "test.h"
#include "src/utils/utils.h"
#include "src/tasks/task.h"

using namespace residue;

class SimpleTask final : public Task
{
public:
    explicit SimpleTask(unsigned int interval) :
        Task("SimpleTask", nullptr, interval)
    {
    }

    virtual void execute() override
    {
        LOG(INFO) << "SimpleTask::execute()";
    }
};

class SimpleTaskWithRoundOff final : public Task
{
public:
    explicit SimpleTaskWithRoundOff(unsigned int interval) :
        Task("SimpleTaskWithRoundOff", nullptr, interval, true)
    {
    }

    virtual void execute() override
    {
        LOG(INFO) << "SimpleTaskWithRoundOff::execute()";
    }
};

TEST(TaskScheduleTest, TestBasicSchedule)
{
    const unsigned long now = Utils::now();
    int interv = 20;
    SimpleTask t(interv);
    ASSERT_EQ(t.nextExecution(), now + interv);
    ASSERT_EQ(t.intervalCount(), interv);

    interv = 3600;
    SimpleTaskWithRoundOff t2(interv);
    unsigned long nearestNextHour = (3600 - (now % 3600));
    ASSERT_EQ(t2.intervalCount(), nearestNextHour + interv);
    ASSERT_EQ(t2.nextExecution(), now + nearestNextHour + interv);

    interv = 300;
    SimpleTaskWithRoundOff t3(interv);
    nearestNextHour = (3600 - (now % 3600));
    ASSERT_EQ(t3.intervalCount(), nearestNextHour + interv);
    ASSERT_EQ(t3.nextExecution(), now + nearestNextHour + interv);
}

#endif // TASK_SCHEDULE_TEST_H