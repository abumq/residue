//
//  crash-handlers.h
//  Residue
//
//  Copyright 2017-present @abumq (Majid Q.)
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

#ifndef CrashHandlers_h
#define CrashHandlers_h

#include "logging/log.h"
#include "utils/utils.h"

using namespace residue;

static bool s_exitOnInterrupt = false;

void interruptHandler(int)
{
    if (s_exitOnInterrupt) {
        el::Helpers::crashAbort(SIGINT);
    } else {
        std::cerr << "(To exit, press ^C again or type quit)" << std::endl;
        s_exitOnInterrupt = true;
    }
}

void generalTerminateHandler(int sig, bool showMsg)
{
    if (showMsg) {
        std::cerr << Utils::formatTime(Utils::now(), "%d/%M/%Y %h:%m:%s") << " ";
        std::cerr << "Application abnormally terminated." << std::endl;
        std::cerr << "Please report it to us on https://github.com/abumq/residue/issues/ " << std::endl;
    }
    el::Helpers::logCrashReason(sig, true, el::Level::Fatal, RESIDUE_LOGGER_ID);
    el::Helpers::crashAbort(sig);
}

void elppCrashHandler(int sig)
{
    generalTerminateHandler(sig, true);
}

void terminateHandler()
{
    generalTerminateHandler(11, false);
}

#endif /* CrashHandlers_h */
