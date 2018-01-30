//
//  log-extension.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef LogExtension_h
#define LogExtension_h

#include <string>
#include "include/log.h"
#include "src/extensions/extension.h"

namespace residue {

class LogExtension : public Extension
{
public:
    LogExtension(const std::string& module);

    bool call(const el::LogDispatchData*,
              const std::string& logLine);
};
}

#endif /* LogExtension_h */
