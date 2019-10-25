//
//  zlib.cc
//  Residue
//
//  Copyright 2017-present Amrayn Web Services
//  https://amrayn.com
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

#include "crypto/zlib.h"

#include "logging/log.h"

#ifdef RESIDUE_USE_MINE
#   include "mine/mine.h"
#else
#   include "ripe/Ripe.h"
#endif

using namespace residue;

std::string ZLib::compress(const std::string& data)
{
#ifdef RESIDUE_USE_MINE
    return mine::ZLib::compressString(data);
#else
    return Ripe::compressString(data);
#endif
}

std::string ZLib::decompress(const std::string& data)
{
#ifdef RESIDUE_USE_MINE
    return mine::ZLib::decompressString(data);
#else
    return Ripe::decompressString(data);
#endif
}

bool ZLib::compressFile(const std::string& gzoutFilename, const std::string& inputFile)
{
#ifdef RESIDUE_USE_MINE
    try {
        return mine::ZLib::compressFile(gzoutFilename, inputFile);
    } catch (const std::exception& e) {
        DRVLOG(RV_ERROR) << "Failed to compress file with zlib " << inputFile << ", e=" << e.what();
    }
    return false;
#else
    try {
        return Ripe::compressFile(gzoutFilename, inputFile);
    } catch (const std::exception& e) {
        DRVLOG(RV_ERROR) << "Failed to compress file with zlib " << inputFile << ", e=" << e.what();
    }
    return false;
#endif
}
