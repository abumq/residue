//
//  task-schedule-test.h
//  Residue
//
//  Copyright 2017-present @abumq (Majid Q.)
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

#ifndef TASK_SCHEDULE_TEST_H
#define TASK_SCHEDULE_TEST_H

#include "test.h"

#include "tasks/task.h"
#include "utils/utils.h"

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

TEST(TaskScheduleTest, TestBasicSchedule)
{
    const types::Time now = Utils::now();
    int interv = 20;
    SimpleTask t(interv);
    t.rescheduleFrom(now);
    ASSERT_EQ(now + interv, t.nextExecution());
    ASSERT_EQ(interv, t.intervalCount());
}

#endif // TASK_SCHEDULE_TEST_H
