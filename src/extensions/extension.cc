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


#if (!defined(RESIDUE_EXTENSION_LIB) && defined(RESIDUE_HAS_EXTENSIONS))
#   define RESIDUE_CORE_EXTENSION_ENABLED
#endif

#ifdef RESIDUE_CORE_EXTENSION_ENABLED
#   include <dlfcn.h>
#   include "logging/log.h"
#endif

using namespace residue;

Extension::Extension(unsigned int type, const std::string& id) :
    m_type(type),
    m_id(id),
    m_running(false)
{
    // note: does not log anything here
}

Extension::~Extension()
{
#ifdef RESIDUE_CORE_EXTENSION_ENABLED
    if (m_running) {
        RVLOG(RV_DEBUG_2) << "Extension [" << m_type << "/" << m_id << "] was running when it was terminated";
    }
    RVLOG(RV_DEBUG_2) << "Terminating extension [" << m_type << "/" << m_id << "]";
#endif
}


Extension::Result Extension::trigger(void* data)
{
#ifdef RESIDUE_CORE_EXTENSION_ENABLED
    if (m_running) {
#   ifdef RESIDUE_DEBUG
        DRVLOG(RV_WARNING) << "Extension [" << m_type << "/" << m_id << "] already running";
#   endif
        return {0, true};
    }
    RVLOG(RV_DEBUG_2) << "Executing extension [" << m_type << "/" << m_id << "]";
    m_running = true;
    std::lock_guard<std::mutex> lock_(m_mutex);
    (void) lock_;
    auto result = executeWrapper(data);
    RVLOG(RV_DEBUG_2) << "Finished execution of extension [" << m_type << "/" << m_id << "]";
    m_running = false;
    return result;
#else
    (void) data;
    return {0, true};
#endif
}


Extension* Extension::load(const char* path)
{
#ifdef RESIDUE_CORE_EXTENSION_ENABLED
    void* handle = dlopen(path, RTLD_LAZY);

    if (handle == nullptr) {
        const char* dlsymError = dlerror();
        RLOG(ERROR) << "Cannot load extension [" << path << "]: " << (dlsymError ? dlsymError : "Unable to extract the error");
        return nullptr;
    }

    using CreateExtensionFn = Extension* (*)();

#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpedantic"

    CreateExtensionFn create = reinterpret_cast<CreateExtensionFn>(dlsym(handle, "create_extension"));

#   pragma GCC diagnostic pop

    if (create == nullptr) {
        RLOG(ERROR) << "Extension failed [" << path << "]. Missing [RESIDUE_EXTENSION] from the extension.";
        return nullptr;
    }

    DRLOG(TRACE) << "Creating extension...";
    Extension* e = create();

    const char* dlsymError = dlerror();

    if (dlsymError) {
        RLOG(ERROR) << "Extension failed [" << path << "]. Hint: did you forget to RESIDUE_EXTENSION? " << dlsymError;
        return nullptr;
    }
    return e;
#else
    (void) path;
    return nullptr;
#endif
}

void Extension::writeLog(const std::string& msg, LogLevel level, unsigned short vlevel) const
{
#ifdef RESIDUE_CORE_EXTENSION_ENABLED
    if (level == LogLevel::Info) {
        RLOG(INFO) << "[Extension <" << m_id << ">] " << msg;
    } else if (level == LogLevel::Error) {
        RLOG(ERROR) << "[Extension <" << m_id << ">] " << msg;
    } else if (level == LogLevel::Warning) {
        RLOG(WARNING) << "[Extension <" << m_id << ">] " << msg;
    } else if (level == LogLevel::Debug) {
        RLOG(DEBUG) << "[Extension <" << m_id << ">] " << msg;
    } else if (level == LogLevel::Trace) {
        RLOG(TRACE) << "[Extension <" << m_id << ">] " << msg;
    } else if (level == LogLevel::Verbose) {
        RVLOG(vlevel) << "[Extension <" << m_id << ">] " << msg;
    }
#else
    (void) msg;
    (void) level;
    (void) vlevel;
#endif
}

#undef RESIDUE_CORE_EXTENSION_ENABLED
