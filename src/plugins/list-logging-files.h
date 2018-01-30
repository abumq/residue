//
//  list-logging-files.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef ListLoggingFiles_h
#define ListLoggingFiles_h

#include "src/plugins/plugin.h"

namespace residue {

class Registry;

///
/// \brief ListLoggingFiles plugin
///
class ListLoggingFiles : public Plugin
{
public:
    ListLoggingFiles(Registry* registry);

    virtual void execute(std::vector<std::string>&&, std::ostringstream&, bool) const override;
private:
    std::string getFile(const std::string& loggerId, const std::string& levelStr) const;

};
}

#endif /* ListLoggingFiles_h */