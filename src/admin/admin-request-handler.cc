//
//  admin-request-handler.cc
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
#include "admin/admin-request-handler.h"
#include "admin/admin-request.h"
#include "core/configuration.h"
#include "cli/command-handler.h"
#include "net/session.h"
#include "tasks/log-rotator.h"

using namespace residue;

AdminRequestHandler::AdminRequestHandler(Registry* registry, CommandHandler* commandHandler) :
    RequestHandler("Admin", registry),
    m_commandHandler(commandHandler)
{
#ifdef RESIDUE_DEV
    DRVLOG(RV_DEBUG) << "AdminRequestHandler " << this << " with registry " << m_registry;
#endif
}

void AdminRequestHandler::handle(RawRequest&& rawRequest)
{
    AdminRequest request(m_registry->configuration());

    RequestHandler::handle(std::move(rawRequest), &request, Request::StatusCode::BAD_REQUEST, false, true);

    if (request.statusCode() != Request::StatusCode::CONTINUE) {
        RLOG(ERROR) << "Unable to continue with this request! " << request.errorText();
        respond("Unable to continue with this request!");
        return;
    }

    if (!request.isValid()) {
        RLOG(ERROR) << "Invalid admin request";
        respond("Invalid admin request");
        return;
    }
    std::string cmd;
    std::vector<std::string> params;
    params.reserve(6); // following pushes maximum of 6 parameters in any case

    switch (request.type()) {
    case AdminRequest::Type::RELOAD_CONFIG:
        cmd = "rconfig";
        if (!request.loggerId().empty()) {
            params.push_back("--logger-id");
            params.push_back(request.loggerId());
        }
        break;
    case AdminRequest::Type::RESET:
        cmd = "reset";
        break;
    case AdminRequest::Type::FORCE_LOG_ROTATION:
        cmd = "rotate";
        params.push_back("--logger-id");
        params.push_back(request.loggerId());
        break;
    case AdminRequest::Type::ADD_CLIENT:
        cmd = "clients";
        params.push_back("add");
        params.push_back(request.clientId());
        params.push_back(request.rsaPublicKey());
        break;
    case AdminRequest::Type::REMOVE_CLIENT:
        cmd = "clients";
        params.push_back("remove");
        params.push_back(request.clientId());
        break;
    case AdminRequest::Type::LIST_CLIENTS:
        cmd = "clients";
        params.push_back("list");
        break;
    case AdminRequest::Type::STATS:
        cmd = "stats";
        params.push_back("list");
        if (!request.clientId().empty()) {
            params.push_back("--client-id");
            params.push_back(request.clientId());
        }
        break;
    case AdminRequest::Type::LIST_LOGGING_FILES:
    {
        cmd = "files";
        params.push_back("--client-id");
        params.push_back(request.clientId());
        std::string loggingLevelsCsv;
        for (std::string l : request.loggingLevels()) {
            loggingLevelsCsv.append(l);
            loggingLevelsCsv.append(",");
        }
        if (!loggingLevelsCsv.empty()) {
            // remove last comma
            loggingLevelsCsv.erase(loggingLevelsCsv.size() - 1);
        }
        if (!loggingLevelsCsv.empty()) {
            params.push_back("--levels");
            params.push_back(loggingLevelsCsv);
        }
        if (!request.loggerId().empty()) {
            params.push_back("--logger-id");
            params.push_back(request.loggerId());
        }
        break;
    }
    case AdminRequest::Type::UNKNOWN:
    default:
        break;
    }

    std::ostringstream result;
    if (!cmd.empty()) {
        std::stringstream fullCmd;
        fullCmd << cmd;
        for (const std::string& p : params) {
            fullCmd << " " << p;
        }
        RVLOG(RV_INFO) << "Running command via admin request: " << fullCmd.str();
        m_commandHandler->handle(std::move(cmd), std::move(params), result, true);
    } else {
        RLOG(ERROR) << "Unknown admin request [" << static_cast<unsigned short>(request.type()) << "]";
        result << "Unknown admin request";
    }
    result << std::endl;
    respond(result.str());
}

void AdminRequestHandler::respond(const std::string& response) const
{
    m_session->write(response.c_str(), configuration()->serverKey().c_str());
}
