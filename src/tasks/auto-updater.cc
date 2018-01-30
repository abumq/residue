//
//  auto-updater.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include "include/log.h"
#include "src/tasks/auto-updater.h"
#include "src/core/registry.h"
#include "src/core/configuration.h"
#include "src/net/url.h"
#include "src/utils/utils.h"
#include "src/net/http-client.h"
#include "src/core/json-object.h"

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
    if (check(&newVer)) {
        RLOG(INFO) << "You are running v" << RESIDUE_VERSION << ", new version is available: " << newVer;
    }
}

bool AutoUpdater::check(std::string* newVersion)
{
    std::string major = RESIDUE_VERSION_MAJOR;
    std::string minor = RESIDUE_VERSION_MINOR;
    std::string patch = RESIDUE_VERSION_PATCH;
    std::string curr = "v" + std::string(RESIDUE_VERSION);
    RLOG(INFO) << "Checking for updates...";
    RVLOG(RV_INFO) << "Current version detected " << curr << " (" << major << "." << minor << "." << patch << ")";

    std::string resultFromApi;
    RVLOG(RV_DEBUG) << "Calling github API...";
    resultFromApi = HttpClient::fetchUrlContents(LATEST_RELEASE_API_URL);
    RVLOG_IF(resultFromApi.empty(), RV_DEBUG) << "Github API returned no result. Connection issue!";
    DRVLOG(RV_DEBUG) << "Github API result: " << resultFromApi;

    try {
        std::string cleanResult = Utils::trim(resultFromApi);
        JsonObject j(std::move(cleanResult));
        if (j.hasKey("tag_name")) {
            *newVersion = j.getString("tag_name");
            return curr != *newVersion;
        } else {
            RLOG(ERROR) << "Invalid JSON returned from github API (expected 'tag_name')\n" << resultFromApi;
        }
    } catch (const std::exception& e) {
        RLOG(ERROR) << "Failed to parse JSON:\n" << e.what() << "\n" << resultFromApi;
    }
    return false;
}