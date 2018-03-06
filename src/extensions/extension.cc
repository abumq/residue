//
//  extension.cc
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

#include "extensions/extension.h"

#ifndef RESIDUE_EXTENSION_LIB
#   include <dlfcn.h>
#   include "logging/log.h"
#endif

using namespace residue;

Extension::Extension(const std::string& type, const std::string& module) :
    m_type(type),
    m_module(module),
    m_running(false)
{

}


Extension::Result Extension::execute(void* data)
{
    if (m_running) {
#ifndef RESIDUE_EXTENSION_LIB
#   ifdef RESIDUE_DEBUG
        DRVLOG(RV_WARNING) << "Extension [" << m_type << "/" << m_module << "] already running";
#   endif
#endif
        return {1, true};
    }
#ifndef RESIDUE_EXTENSION_LIB
    RVLOG(RV_INFO) << "Executing extension [" << m_type << "/" << m_module << "]";
#endif
    m_running = true;
    std::lock_guard<std::mutex> lock_(m_mutex);
    (void) lock_;
    auto result = process(data);
    m_running = false;
    return result;
}


Extension* Extension::load(const char* name)
{
#ifndef RESIDUE_EXTENSION_LIB
    void* handle = dlopen(name, RTLD_LAZY);

    if (handle == nullptr) {
        return nullptr;
    }

    Extension* (*create)();

    create = (Extension* (*)())dlsym(handle, "create_extension");

    return create();
#else
    (void) name;
    return nullptr;
#endif
}
