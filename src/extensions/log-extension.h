//
//  log-extension.h
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

#ifndef LogExtension_h
#define LogExtension_h

#include <string>
#include "src/logging/log.h"
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
