//
//  auto-updater.cc
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

#include "logging/log.h"
#include "tasks/auto-updater.h"
#include "core/registry.h"
#include "core/configuration.h"
#include "net/url.h"
#include "utils/utils.h"
#include "net/http-client.h"
#ifdef RESIDUE_USE_GASON
#include "core/json-doc.h"
#else
#include "core/json-document.h"
#endif

using namespace residue;

const std::string AutoUpdater::LATEST_RELEASE_API_URL = "https://api.github.com/repos/muflihun/residue/releases/latest";

AutoUpdater::AutoUpdater(Registry* registry,
                       unsigned int interval) :
    Task("AutoUpdater", registry, interval)
{

}

void AutoUpdater::execute()
{
    std::string newVer;
    if (hasNewVersion(&newVer)) {
        RVLOG(RV_NOTICE) << "You are running Residue v" << RESIDUE_VERSION
                         << ", a newer version is available: " << newVer
                         << ". Please visit https://github.com/muflihun/residue/releases/tag/" << newVer;
    }
}

bool AutoUpdater::hasNewVersion(std::string* newVersion)
{
    std::string major = RESIDUE_VERSION_MAJOR;
    std::string minor = RESIDUE_VERSION_MINOR;
    std::string patch = RESIDUE_VERSION_PATCH;
    std::string curr = "v" + std::string(RESIDUE_VERSION);
    RLOG(INFO) << "Checking for updates...";
    RVLOG(RV_INFO) << "Current version detected " << curr << " (" << major << "." << minor << "." << patch << ")";

    std::string resultFromApi;
    RVLOG(RV_DEBUG) << "Making github API call...";
    resultFromApi = HttpClient::fetchUrlContents(LATEST_RELEASE_API_URL);
    RVLOG_IF(resultFromApi.empty(), RV_DEBUG) << "Github API returned no result. Connection issue!";
    DRVLOG(RV_CRAZY) << "Github API result: " << resultFromApi;

    try {
        std::string cleanResult = Utils::trim(resultFromApi);
#ifdef RESIDUE_USE_GASON
        JsonDoc j;
        j.parse(cleanResult);
        if (j.hasKey("tag_name")) {
            *newVersion = j.get<std::string>("tag_name", "");
            return curr != *newVersion;
        } else {
            RLOG(ERROR) << "Invalid JSON returned from github API (expected 'tag_name')\n" << resultFromApi;
        }
#else
        JsonDocument j(std::move(cleanResult));
        if (j.hasKey("tag_name")) {
            *newVersion = j.getString("tag_name");
            return curr != *newVersion;
        } else {
            RLOG(ERROR) << "Invalid JSON returned from github API (expected 'tag_name')\n" << resultFromApi;
        }
#endif
    } catch (const std::exception& e) {
        RLOG(ERROR) << "Failed to parse JSON:\n" << e.what() << "\n" << resultFromApi;
    }
    return false;
}
