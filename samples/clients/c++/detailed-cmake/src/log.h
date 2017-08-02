#ifndef LOG_H
#define LOG_H
// THESE TWO LINES ARE IMPORTANT TO TELL EASYLOGGING++ IN THIS FILE
// WHEN WE USE LOG(...) AND PERFORMANCE TRACKING IT SHOULD
// USE muflihun LOGGER RATHER THAN default LOGGER
#define ELPP_DEFAULT_LOGGER "sample-app"
#define ELPP_DEFAULT_PERFORMANCE_LOGGER "sample-app"

#ifdef NON_ROOT_SAMPLE_BUILD // In your application you would do #include <Residue.h>
#      include <Residue.h>
#   else
#      include "include/Residue.h"
#endif

#endif // LOG_H
