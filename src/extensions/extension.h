//
//  extension.h
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

#ifndef Extension_h
#define Extension_h

#include <atomic>
#include <mutex>
#include <string>

namespace residue {
class Extension
{
public:
    struct Result {
        int statusCode;
        bool continueProcess;
    };

    Extension(const std::string& type, const std::string& module);

    virtual ~Extension() {}

    static Extension* load(const char*);

protected:
    virtual Result process(void*) {
        return {0, true};
    }
private:
    std::string m_type;
    std::string m_module;
    std::atomic<bool> m_running;
    std::mutex m_mutex;

    Result execute(void*);

    friend class ResidueLogDispatcher;
};

}

#ifdef RESIDUE_EXTENSION_LIB
#define RESIDUE_EXTENSION(Name, Version)\
    extern "C" Name* create_extension()\
    {\
        static Name singl;\
        return &singl;\
    }
#endif

#endif /* Extension_h */
