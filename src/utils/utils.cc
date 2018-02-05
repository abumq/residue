//
//  utils.cc
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

#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <string>
#include <random>
#include <algorithm>
#include "src/logging/log.h"
#include "src/core/configuration.h"
#include "src/core/residue-exception.h"
#include "src/utils/tar.h"
#include "src/net/url.h"
#include "src/utils/utils.h"

using namespace residue;

const std::string Utils::SIZE_UNITS[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

std::string& Utils::replaceFirstWithEscape(std::string& str,
                                           const std::string& replaceWhat,
                                           const std::string& replaceWith,
                                           char formatSpecifierChar)
{
    std::size_t foundAt = std::string::npos;
    while ((foundAt = str.find(replaceWhat, foundAt + 1)) != std::string::npos) {
        if (foundAt > 0 && str[foundAt - 1] == formatSpecifierChar) {
            str.erase(foundAt > 0 ? foundAt - 1 : 0, 1);
            ++foundAt;
          } else {
            str.replace(foundAt, replaceWhat.length(), replaceWith);
            break;
        }
    }
    return str;
}

std::string& Utils::replaceAll(std::string &str,
                               const std::string &replaceWhat,
                               const std::string &replaceWith,
                               int incr,
                               bool forceFull)
{
    if (replaceWhat == replaceWith) {
      return str;
    }

    int count = 0;
    std::size_t foundAt = std::string::npos;
    while ((foundAt = str.find(replaceWhat, foundAt + incr)) != std::string::npos) {
      str.replace(foundAt, replaceWhat.length(), replaceWith);
      if (!forceFull && count++ == 5000) {
          str = "error Utils::replaceAll => <too many occurrences>. See <incr> parameter";
          return str;
      }
    }
    return str;
}

std::string& Utils::bigAdd(std::string& dest,
                           std::string&& src)
{
    char carry = 0;
    std::size_t s1 = dest.size(), s2 = src.size(), n = std::max(s1, s2);
    if (n > s1) {
        dest = std::string(n - s1, '0') + dest;
    } else if (n > s2) {
        src = std::string(n - s2, '0') + src;
    }

    std::string result(n + 1, '0');

    std::transform(dest.rbegin(), dest.rend(), src.rbegin(), result.rbegin(), [&carry]( char x,  char y){
        char z = (x - '0') + (y - '0') + carry;
        if (z > 9) {
            carry = 1;
            z -= 10;
        } else {
            carry = 0;
        }
        return z + '0';
    });

    result[0] = carry + '0';

    //Remove the leading zeros
    n = result.find_first_not_of("0");
    if (n != std::string::npos) {
        result = result.substr(n);
    }
    dest = result;
    return dest;
}

bool Utils::fileExists(const char* path)
{
    if (path == nullptr) {
        return false;
    }
    struct stat buf;
    return (stat(path, &buf) == 0);
}

bool Utils::createPath(const std::string& path, unsigned int mode)
{
    if (path.empty()) {
        return false;
    }
    if (Utils::fileExists(path.c_str())) {
        return true;
    }
    int status = -1;
    std::size_t foundAt = std::string::npos;
    std::string pathCopy(path);
    pathCopy.append("/"); // append it in the end there is no harm but it will be used to find
    std::string partialPath;
    while ((foundAt = pathCopy.find_first_of("/")) != std::string::npos) {
        partialPath.append(pathCopy.substr(0, foundAt + 1));
        if (pathCopy != "/") {
            status = mkdir(partialPath.c_str(), mode);
        } else {
            status = 0;
        }
        pathCopy = pathCopy.substr(foundAt + 1);
    }

    if (status == -1) {
        return false;
    } else {
        chmod(path.c_str(), mode);
    }
    return true;
}

long Utils::fileSize(const char* filename)
{
    if (filename == nullptr) {
        return 0;
    }
    struct stat stat_buf;
    int rc = stat(filename, &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

void Utils::updateFilePermissions(const char* path, const el::Logger* logger, const Configuration* conf)
{
    chmod(path, conf->fileMode());

    // find owner for file
    const std::string fileUser = conf->findLoggerUser(logger->id());
    if (!fileUser.empty()) {
        struct passwd* userDetails = getpwnam(fileUser.data());
        if (userDetails == nullptr) {
            RLOG(ERROR) << "User [" << fileUser << "] does not exist. Unable to change ownership for " << path;
            endpwent();
            return;
        } else {
            RVLOG(RV_INFO) << "Changing ownership for [" << path << "] to [" << fileUser << "]";
        }
        uid_t userId = userDetails->pw_uid;
        gid_t groupId = userDetails->pw_gid;
        endpwent();
        if (chown(path, userId, groupId) == -1) {
            RLOG(ERROR) << "Failed to change ownership for " << path << ". Error: " << strerror(errno);
        }
    } else {
        RVLOG(RV_INFO) << "No config user found for [" << path << "]; logger ["
                       << (logger == nullptr ? "NULL" : logger->id()) << "]";
    }
}

std::string Utils::generateRandomFromArray(const char* list,
                                           std::size_t size,
                                           unsigned int length)
{
    std::string s(length, 'x');
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> uni(0, size - 1);
    std::generate(s.begin(), s.end(), [&] {
        return list[uni(rng)];
    });
    return s;
}

std::string& Utils::ltrim(std::string& str)
{
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](char c) {
        return !std::isspace(c);
    } ));
    return str;
}

std::string& Utils::rtrim(std::string& str)
{
    str.erase(std::find_if(str.rbegin(), str.rend(), [](char c) {
        return !std::isspace(c);
    }).base(), str.end());
    return str;
}

bool Utils::isJSON(const std::string& data)
{
    if (data.empty()) {
        return false;
    }
    std::string copy(data);
    Utils::ltrim(copy);
    return copy[0] == '[' || copy[0] == '{';
}

bool Utils::archiveFiles(const std::string& outputFile,
                         const std::map<std::string, std::string>& files)
{
    std::fstream out(outputFile, std::ios::out);
    if (!out.is_open()) {
        RLOG(ERROR) << "Unable to open file [" << outputFile << "] for writing. " << std::strerror(errno);
        return false;
    }
    bool result = true;
    Tar tar(out);
    for (auto f : files) {
        if (!tar.putFile(f.first.c_str(), f.second.c_str())) {
            result = result && false;
        }
    }
    tar.finish();
    out.close();
    return result;
}

std::string Utils::bytesToHumanReadable(long size)
{
    int index = 0;
    float fSize = static_cast<float>(size);
    while (fSize > 1024L) {
        fSize /= 1024L;
        index++;
    }
    std::stringstream ss;
    if (fSize / static_cast<int>(fSize) > 1.0f) {
        ss << std::fixed << std::setprecision(1) << fSize << SIZE_UNITS[index];
    } else {
        ss << static_cast<long>(fSize) << SIZE_UNITS[index];
    }
    return ss.str();
}

unsigned long long Utils::nowUtc()
{
    time_t t = std::time(nullptr);
    tm* nowTm;
    nowTm = std::gmtime(&t);
    return nowTm != nullptr ? mktime(nowTm) : 0;
}

std::string Utils::formatTime(unsigned long time, const char* format)
{
    timeval t;
    t.tv_sec = time;
    el::base::SubsecondPrecision ssPrec(3);
    return el::base::utils::DateTime::timevalToString(t, format, &ssPrec);
}

tm Utils::timeToTm(unsigned long epochInSec)
{
    timeval tval;
    tval.tv_sec = epochInSec;
    tm tmObj;
    el::base::utils::DateTime::buildTimeInfo(&tval, &tmObj);
    return tmObj;
}
