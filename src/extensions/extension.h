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
#include <thread>
#include <mutex>
#include <queue>
#include <string>

namespace residue {

class Extension
{
    static std::mutex s_extensionMutex;
public:
    Extension(const std::string& module, const std::string& func);

    virtual ~Extension();

    inline std::string module() const
    {
        return m_module;
    }

    inline std::string func() const
    {
        return m_func;
    }

private:
    std::string m_module;
    std::string m_func;
    std::atomic<bool> m_running;
    std::thread m_worker;
    std::mutex m_mutex;
    std::queue<std::string> m_scripts;
protected:
    void escape(std::string& str) const;
    void executeScript(const std::string& script);
    void work();
};
}


#endif /* Extension_h */
