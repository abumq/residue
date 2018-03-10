#include "failure-notify.h"

RESIDUE_EXTENSION(FailureNotify, "1.0.0")

Extension::Result FailureNotify::execute(const DispatchErrorExtension::Data* const data)
{
    if (!data) {
        writeLog("Data not passed in", Extension::LogLevel::Error);
        return {1, true};
    }

    return {0, true};
}
