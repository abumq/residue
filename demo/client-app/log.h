#ifndef LOG_H
#define LOG_H

#define ELPP_DEFAULT_LOGGER "default"
#define ELPP_DEFAULT_PERFORMANCE_LOGGER "default"

#include <Residue.h>

void displayErrors()
{
    if (!Residue::errors().empty()) {
        std::cout << "Last 10 Errors from the server (latest is last):" << std::endl;
        for (std::string e : Residue::errors()) {
            std::cout << e << std::endl;
        }
    } else {
        std::cout << "No errors recorded!" << std::endl;
    }
}

#endif // LOG_H

