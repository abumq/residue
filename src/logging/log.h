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

#ifdef RESIDUE_DEBUG
#   define RESIDUE_DEBUG_LOG 1
#else
#   define RESIDUE_DEBUG_LOG 0
#endif // RESIDUE_DEBUG

#if 1
#   define RLOG(LEVEL) CLOG(LEVEL, RESIDUE_LOGGER_ID)
#   define RLOG_IF(condition, LEVEL) CLOG_IF(condition, LEVEL, RESIDUE_LOGGER_ID)
#   define RVLOG(vLevel) CVLOG(vLevel, RESIDUE_LOGGER_ID)
#   define RVLOG_IF(cond, vLevel) CVLOG_IF(cond, vLevel, RESIDUE_LOGGER_ID)
#   define DRLOG(LEVEL) if (RESIDUE_DEBUG_LOG) DCLOG(LEVEL, RESIDUE_LOGGER_ID)
#   define DRVLOG(vLevel) if (RESIDUE_DEBUG_LOG) DCVLOG(vLevel, RESIDUE_LOGGER_ID)
#   define DRVLOG_IF(condition, vLevel) if (RESIDUE_DEBUG_LOG) DCVLOG_IF(condition, vLevel, RESIDUE_LOGGER_ID)
#else
#   define RLOG(LEVEL) el::base::NullWriter()
#   define RLOG_IF(condition, LEVEL) el::base::NullWriter()
#   define RVLOG(vLevel) el::base::NullWriter()
#   define RVLOG_IF(cond, vLevel) el::base::NullWriter()
#   define DRLOG(LEVEL) el::base::NullWriter()
#   define DRVLOG(vLevel) el::base::NullWriter()
#   define DRVLOG_IF(condition, vLevel) el::base::NullWriter()
#endif

#define RV_CRAZY 9
#define RV_TRACE 8
#define RV_DEBUG_2 7
#define RV_DEBUG 6
#define RV_DETAILS 5
#define RV_WARNING 4
#define RV_ERROR 3
#define RV_NOTICE 2
#define RV_INFO 1

#define RESIDUE_UNUSED(x) (void)x

#ifdef RESIDUE_PROFILING
#   define RESIDUE_PROFILE_CHECKPOINT_TEMPL(id, result, idx, previdx, type, unit) std::chrono::high_resolution_clock::time_point id##chkpnt_3##idx = std::chrono::high_resolution_clock::now();\
       result = std::chrono::duration_cast<type>( id##chkpnt_3##idx - id##1 ).count();\
       {auto resultforcompar = std::chrono::duration_cast<type>( id##chkpnt_3##idx - id##chkpnt_3##previdx ).count();\
       RLOG(DEBUG) << #id << idx << " from base " << result << " " << unit << " (from prev checkpoint " << resultforcompar << " " << unit ")";}
#   define RESIDUE_PROFILE_START(id) std::chrono::high_resolution_clock::time_point id##1 = std::chrono::high_resolution_clock::now(); RESIDUE_UNUSED(id##1)
#   define RESIDUE_PROFILE_END(id, result) std::chrono::high_resolution_clock::time_point id##2 = std::chrono::high_resolution_clock::now();\
       result = std::chrono::duration_cast<std::chrono::milliseconds>( id##2 - id##1 ).count();
#   define RESIDUE_PROFILE_CHECKPOINT(id, result, idx, previdx) RESIDUE_PROFILE_CHECKPOINT_TEMPL(id, result, idx, previdx, std::chrono::milliseconds, "ms")
#   define RESIDUE_PROFILE_CHECKPOINT_NS(id, result, idx, previdx) RESIDUE_PROFILE_CHECKPOINT_TEMPL(id, result, idx, previdx, std::chrono::nanoseconds, "nans")
#   define RESIDUE_PROFILE_CHECKPOINT_MIS(id, result, idx, previdx) RESIDUE_PROFILE_CHECKPOINT_TEMPL(id, result, idx, previdx, std::chrono::microseconds, "mis")

#else
#   define RESIDUE_PROFILE_START(id)
#   define RESIDUE_PROFILE_END(id, result)
#   define RESIDUE_PROFILE_CHECKPOINT(id, result, idx, previdx)
#   define RESIDUE_PROFILE_CHECKPOINT_MIS(id, result, idx, previdx)
#   define RESIDUE_PROFILE_CHECKPOINT_NS(id, result, idx, previdx)
#endif // RESIDUE_PROFILING

#if defined(RESIDUE_DEV) && defined(RESIDUE_PROFILING)
#define RESIDUE_HIGH_RESOLUTION_PROFILING
#endif

#endif // LOG_H
