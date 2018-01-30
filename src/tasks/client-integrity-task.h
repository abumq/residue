//
//  client-integrity-task.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef ClientIntegrityTask_h
#define ClientIntegrityTask_h

#include "src/tasks/task.h"

namespace residue {

class Registry;

///
/// \brief Task to check integrity of the clients (and their tokens).
/// If invalid, this task will remove it from the registry
///
class ClientIntegrityTask : public Task
{
public:
    explicit ClientIntegrityTask(Registry* registry,
                                 unsigned int interval);
protected:
    virtual void execute() override;
};
}
#endif /* ClientIntegrityTask_h */