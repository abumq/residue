//
//  log-extension.cc
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

#include "extensions/log-extension.h"
#include "logging/user-log-builder.h"
#include "logging/log-request.h"

using namespace residue;

LogExtension::LogExtension(const std::string &module)
    : Extension(module, "handle_log")
{

}

bool LogExtension::call(const el::LogDispatchData* data,
                        const std::string& logLine)
{
#ifdef RESIDUE_HAS_EXTENSIONS
    std::stringstream ss;
    UserLogBuilder* logBuilder = static_cast<UserLogBuilder*>(data->logMessage()->logger()->logBuilder());
    if (logBuilder == nullptr) {
        return false;
    }
    std::string threadId = logBuilder->request()->threadId();
    std::string clientId = logBuilder->request()->clientId();
    std::string applicationName = logBuilder->request()->applicationName();
    types::TimeMs datetime = logBuilder->request()->datetime();
    std::string threadName = logBuilder->request()->threadName();
    std::string msg = data->logMessage()->message();
    std::string sourceFile = data->logMessage()->file();
    std::string logLineCopy(logLine);
    escape(logLineCopy);
    escape(msg);
    escape(threadName);
    escape(applicationName);
    escape(threadId);
    escape(clientId);
    escape(sourceFile);

    ss << "import " << module() << "\n";
    ss << module() << "." << func() << "(";
    ss << "'" << logLineCopy << "',"
       << "'" << clientId    << "',"
       << "'" << data->logMessage()->logger()->id() << "',"
       << datetime << ","
       << "'" << el::LevelHelper::convertToString(data->logMessage()->level()) << "',"
       << "'" << data->logMessage()->verboseLevel() << "',"
       << "'" << sourceFile << "',"
       << data->logMessage()->line() << ","
       << "'" << data->logMessage()->func() << "',"
       << "'" << msg << "',"
       << "'" << threadName << "',"
       << "'" << threadId << "',"
       << "'" << applicationName << "'"
       << ")\n";
    executeScript(ss.str());
#else
    (void)data;
    (void)logLine;
#endif
    return true;
}
