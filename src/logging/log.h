//
//  log.h
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

#ifndef LOG_H
#define LOG_H

#include "easyloggingpp/easylogging++.h"

#define RESIDUE_LOGGER_ID "residue"

#define RLOG(LEVEL) CLOG(LEVEL, RESIDUE_LOGGER_ID)
#define RLOG_IF(condition, LEVEL) CLOG_IF(condition, LEVEL, RESIDUE_LOGGER_ID)
#define RVLOG(vLevel) CVLOG(vLevel, RESIDUE_LOGGER_ID)
#define RVLOG_IF(cond, vLevel) CVLOG_IF(cond, vLevel, RESIDUE_LOGGER_ID)
#define DRLOG(LEVEL) if (RESIDUE_DEBUG) DCLOG(LEVEL, RESIDUE_LOGGER_ID)
#define DRVLOG(vLevel) if (RESIDUE_DEBUG) DCVLOG(vLevel, RESIDUE_LOGGER_ID)
#define DRVLOG_IF(condition, vLevel) if (RESIDUE_DEBUG) DCVLOG_IF(condition, vLevel, RESIDUE_LOGGER_ID)

#define RV_CRAZY 9
#define RV_TRACE 8
#define RV_DEBUG 7
#define RV_DETAILS 6
#define RV_5 5
#define RV_WARNING 4
#define RV_ERROR 3
#define RV_NOTICE 2
#define RV_INFO 1

#define RESIDUE_UNUSED(x) (void)x

#ifdef RESIDUE_PROFILING
#define RESIDUE_PROFILE_START(id) std::chrono::high_resolution_clock::time_point id##1 = std::chrono::high_resolution_clock::now(); RESIDUE_UNUSED(id##1)
#define RESIDUE_PROFILE_END(id, result) std::chrono::high_resolution_clock::time_point id##2 = std::chrono::high_resolution_clock::now();\
    result = std::chrono::duration_cast<std::chrono::milliseconds>( id##2 - id##1 ).count();
#define RESIDUE_PROFILE_CHECKPOINT(id, result, idx) std::chrono::high_resolution_clock::time_point id##3##idx = std::chrono::high_resolution_clock::now();\
    result = std::chrono::duration_cast<std::chrono::milliseconds>( id##3##idx - id##1 ).count();\
    std::cout << idx << " checkpoint at " << result << " ms\n";
#else
#define RESIDUE_PROFILE_START(id)
#define RESIDUE_PROFILE_END(id, result)
#define RESIDUE_PROFILE_CHECKPOINT(id, result, idx)
#endif

#endif // LOG_H
