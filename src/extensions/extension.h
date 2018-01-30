//
//  extension.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
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