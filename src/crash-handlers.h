#include "logging/log.h"

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
        std::cerr << "Application abnormally terminated." << std::endl;
        std::cerr << "Please report it to us on https://github.com/muflihun/residue/issues" << std::endl;
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
