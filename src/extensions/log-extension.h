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
#include "logging/log.h"
#include "extensions/extension.h"

namespace residue {

class LogExtension final : public Extension
{
public:
    explicit LogExtension(const std::string& module);

    virtual bool process() override;
};
}

#endif /* LogExtension_h */
