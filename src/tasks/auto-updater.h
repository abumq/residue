//
//  auto-updater.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef AutoUpdater_h
#define AutoUpdater_h

#include "src/tasks/task.h"
#include "src/core/json-object.h"

namespace residue {

class Registry;

///
/// \brief Auto updater task
///
class AutoUpdater final : public Task
{
public:
    static const std::string LATEST_RELEASE_API_URL;

    explicit AutoUpdater(Registry* registry,
                        unsigned int interval);
    bool check(std::string* newVersion);
protected:
    virtual void execute() override;

};
}
#endif /* AutoUpdater_h */