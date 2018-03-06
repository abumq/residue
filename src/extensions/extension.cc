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

#include <dlfcn.h>

#include "utils/utils.h"

using namespace residue;

Extension::Extension(const std::string& name, const std::string& module) :
    m_name(name),
    m_module(module),
    m_running(false)
{

}


bool Extension::execute()
{
    if (m_running) {
        RLOG(WARNING) << "Extension [" << m_name << "] already running";
        return false;
    }
    RLOG(WARNING) << "Executing extension [" << m_name << "]";
    m_running = true;
    std::lock_guard<std::mutex> lock_(m_mutex);
    (void) lock_;
    return process();
}


Extension* Extension::load(const char* name)
{
  void* handle = dlopen(name, RTLD_LAZY);

  Extension* (*create)();

  create = (Extension* (*)())dlsym(handle, "create_extension");

  return create();
}
