//
//  extension.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include "src/extensions/extension.h"
#include "include/Python.h"
#include "src/utils/utils.h"

using namespace residue;

std::mutex Extension::s_extensionMutex;

Extension::Extension(const std::string& module, const std::string& func) :
    m_module(module),
    m_func(func),
    m_running(true)
{
#ifdef RESIDUE_HAS_EXTENSIONS
    Py_Initialize();
    PyEval_InitThreads();
    m_worker = std::thread(&Extension::work, this);
#endif
}

Extension::~Extension()
{
    m_running = false;
#ifdef RESIDUE_HAS_EXTENSIONS
    if (m_worker.joinable()) {
        m_worker.join();
    }
    PyGILState_Ensure();
    Py_Finalize();
#endif
}

void Extension::escape(std::string& text) const
{
    Utils::replaceAll(text, "\r", "\\r");
    Utils::replaceAll(text, "\n", "\\n");
    Utils::replaceAll(text, "'", "\\'", 2);
}

void Extension::executeScript(const std::string &script)
{
    std::lock_guard<std::mutex> lock_(m_mutex);
    (void) lock_;
    m_scripts.push(script);
}

void Extension::work()
{
    el::Helpers::setThreadName("Extension::" + m_module);
#ifdef RESIDUE_HAS_EXTENSIONS
    while (m_running) {
        if (!m_scripts.empty()) {
            std::string script;
            {
                std::lock_guard<std::mutex> lock_(m_mutex);
                (void) lock_;
                script = m_scripts.front();
                m_scripts.pop();
            }
            if (!script.empty()) {
                std::lock_guard<std::mutex> lock_(s_extensionMutex);
                (void) lock_;
#               if RESIDUE_DEBUG
                    DRVLOG(RV_CRAZY) << "Executing extension " << m_module;
#               endif
                int result = PyRun_SimpleString(script.c_str());
                RLOG_IF(result != 0, WARNING) << "Extension ["
                                              << m_module << "] exited with ["
                                              << result << "]";
#               if RESIDUE_DEBUG
                    DRVLOG(RV_CRAZY) << "Finished extension " << m_module;
#               endif
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
#endif
}
