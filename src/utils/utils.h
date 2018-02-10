//
//  utils.h
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

#ifndef Utils_h
#define Utils_h

#include <string>
#include <map>
#include <chrono>
#include "logging/log.h"
#include "static-base.h"
#include "core/types.h"

namespace residue {

class Configuration;

///
/// \brief Contains static utility functions used by Residue
///
class Utils final : StaticBase
{
public:

    static const std::string SIZE_UNITS[];

    // string

    static std::string& replaceFirstWithEscape(std::string& str, const std::string& replaceWhat, const std::string& replaceWith, char formatSpecifierChar = '%');
    static std::string& replaceAll(std::string& str, const std::string& replaceWhat, const std::string& replaceWith, int incr = 1, bool forceFull = false);
    static std::string& ltrim(std::string& str);
    static std::string& rtrim(std::string& str);

    static inline std::string& trim(std::string& str)
    {
        return ltrim(rtrim(str));
    }

    static inline bool isAlphaNumeric(const std::string& str, const std::string& exceptions = "")
    {
        return str.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789" + exceptions) == std::string::npos;
    }

    static inline bool startsWith(const std::string& str, const std::string& start)
    {
        return (str.length() >= start.length()) && (str.compare(0, start.length(), start) == 0);
    }

    static inline bool endsWith(const std::string& str, const std::string& end)
    {
        return (str.length() >= end.length()) && (str.compare(str.length() - end.length(), end.length(), end) == 0);
    }

    static inline std::string generateRandomString(unsigned int size, bool includeUpperCase = false)
    {
        std::string list = includeUpperCase ? "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" : "abcdefghijklmnopqrstuvwxyz";
        return Utils::generateRandomFromArray(list.c_str(), list.size(), size);
    }

    static inline std::string generateRandomInt(unsigned int size)
    {
        return Utils::generateRandomFromArray("0123456789", 10, size);
    }

    static inline std::string& toUpper(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
    }

    static inline std::string& toLower(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }

    // add strings mathematically
    static std::string& bigAdd(std::string& dest, std::string&& src);

    // file
    static bool fileExists(const char* path);
    static bool createPath(const std::string& path, unsigned int mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    static long fileSize(const char* filename);
    static void updateFilePermissions(const char* path, const el::Logger* logger, const Configuration* conf);
    static std::string bytesToHumanReadable(long size);

    // compression
    static bool archiveFiles(const std::string& outputFile, const std::map<std::string, std::string>& files);

    // date
    static inline types::Time now()
    {
        return std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
    }

    static types::Time nowUtc();

    static std::string formatTime(types::Time time, const char* format = "%h:%m:%s");

    static std::tm timeToTm(types::Time epochInSec);

    // serization
    static bool isJSON(const std::string& data);

    static std::string generateRandomFromArray(const char* list, std::size_t size, unsigned int length);
};
}
#endif /* Utils_h */
