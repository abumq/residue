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

#include "core/json-doc.h"

#ifdef RESIDUE_EXTENSION_WIN
#   ifdef  RESIDUE_EXTENSION_LIB
#      define RESIDUE_EXTENSION_API __declspec(dllexport)
#   else
#      define RESIDUE_EXTENSION_API __declspec(dllimport)
#   endif
#else
#   define RESIDUE_EXTENSION_API
#endif

namespace residue {

///
/// \brief Please extend one of the
///
class RESIDUE_EXTENSION_API Extension
{
public:
    struct Result
    {
        int statusCode;
        bool continueProcess;
    };

    Extension(unsigned int type, const std::string& id);

    virtual ~Extension() = default;

protected:
    ///
    /// \brief You will override this function for actual work
    ///
    /// Do not forget to use both 'virtual' and 'override' modifiers
    ///
    virtual Result execute(void*)
    {
        return {0, true};
    }

    ///
    /// \brief Constant access to configurations for this extension
    ///
    inline const JsonDoc& conf() const
    {
        return m_config;
    }

    void writeLog(const std::string&) const;
private:
    unsigned int m_type;
    std::string m_id;
    std::atomic<bool> m_running;
    std::mutex m_mutex;
    JsonDoc m_config;

    friend class ResidueLogDispatcher;
    friend class LogRotator;
    friend class Configuration;

    Result trigger(void*);

    inline void setConfig(JsonDoc::Value&& j)
    {
        m_config.set(j);
    }

    static Extension* load(const char*);
};

}

#define RESIDUE_EXTENSION(Name, Version)\
    extern "C" RESIDUE_EXTENSION_API Name* create_extension()\
    {\
        static Name singl;\
        return &singl;\
    }

#endif /* Extension_h */
