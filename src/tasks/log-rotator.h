//
//  log-rotator.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef LogRotator_h
#define LogRotator_h

#include <unordered_map>
#include <vector>
#include <tuple>
#include "src/tasks/task.h"

namespace residue {

class Registry;

///
/// \brief Log rotator task
///
class LogRotator final : public Task
{
public:
    ///
    /// \brief If log rotator is running and frequency is this threshold (in seconds) away,
    /// it will run it anyway
    ///
    static const unsigned long LENIENCY_THRESHOLD;

    using ArchiveItem = std::tuple<std::string, std::string, std::map<std::string, std::string>>;

    explicit LogRotator(Registry* registry,
                        unsigned int interval);
    void rotate(const std::string& loggerId);
    void archiveRotatedItems();
    std::string checkStatus(const std::string& loggerId);
protected:
    virtual void execute() override;
private:
    void archiveAndCompress(const std::string&,
                            const std::string&,
                            const std::map<std::string, std::string>&);

    std::vector<ArchiveItem> m_archiveItems;
    std::unordered_map<std::string, unsigned long> m_lastRotation;
};
}
#endif /* LogRotator_h */