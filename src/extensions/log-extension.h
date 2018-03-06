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
#include "extensions/extension.h"

namespace residue {

class LogExtension : public Extension
{
public:
    struct Data {
        unsigned int level;
        std::string app;
        std::string thread;
        std::string file;
        unsigned long int line;
        std::string func;
        unsigned short verboseLevel;
        std::string loggerId;
        std::string clientId;
        std::string ipAddr;
        std::string sessionId;
        std::string message;
        std::string formattedMessage;
    };

    explicit LogExtension(const std::string& module);

    virtual Extension::Result process(void*) override;
};
}

#endif /* LogExtension_h */
