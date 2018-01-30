//
//  admin-request-handler.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include "include/log.h"
#include "src/admin/admin-request-handler.h"
#include "src/admin/admin-request.h"
#include "src/core/configuration.h"
#include "src/core/command-handler.h"
#include "src/net/session.h"
#include "src/tasks/log-rotator.h"

using namespace residue;

AdminRequestHandler::AdminRequestHandler(Registry* registry, CommandHandler* commandHandler) :
    RequestHandler(registry),
    m_commandHandler(commandHandler)
{
    DRVLOG(RV_DEBUG) << "AdminRequestHandler " << this << " with registry " << m_registry;
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

    switch (request.type()) {
    case AdminRequest::Type::RELOAD_CONFIG:
        cmd = "reload";
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
        for (std::string p : params) {
            fullCmd << " " << p;
        }
        RLOG(INFO) << "Running command via admin request: " << fullCmd.str();
        m_commandHandler->handle(std::move(cmd), std::move(params), result, true);
    } else {
        RLOG(ERROR) << "Unknown admin request";
        result << "Unknown admin request";
    }
    respond(result.str());
#if 0
    return;

    switch (request.type()) {
    case AdminRequest::Type::RELOAD_CONFIG:
    {
        Configuration tmpConf(configuration()->configurationFile());
        if (tmpConf.isValid()) {
            RLOG(INFO) << "Reloading configurations...";
            configuration()->reload();
            respond("Successfully reloaded configuration");
        } else {
            RLOG(ERROR) << "FAILED to reload configuration. There are errors in configuration file" << std::endl << tmpConf.errors();
            if (tmpConf.isMalformedJson()) {
                respond("FAILED: Malformed JSON in configuration file");
            } else {
                respond("FAILED: There are errors in configuration file\n" + tmpConf.errors());
            }
        }
        break;
    }
    case AdminRequest::Type::RESET:
    {
        registry()->reset();
        respond("Done");
        break;
    }
    case AdminRequest::Type::ADD_CLIENT:
    {
        if (Utils::fileExists(request.rsaPublicKey().c_str())) {
            if (Utils::isAlphaNumeric(request.clientId(), "-_@#")) {
                bool result = configuration()->addKnownClient(request.clientId(), request.rsaPublicKey());
                result = result && configuration()->save(configuration()->configurationFile());
                respond(result ? "Done" : "Failed");
            } else {
                respond("Invalid character in client ID, should be alpha-numeric (can also include these characters excluding square brackets: [_@-#])");
            }
        } else {
            respond("Public key file does not exist on the server");
        }
        break;
    }
    case AdminRequest::Type::REMOVE_CLIENT:
    {
        if (configuration()->isKnownClient(request.clientId())) {
            configuration()->removeKnownClient(request.clientId());
            bool result = configuration()->save(configuration()->configurationFile());
            respond(result ? "Done" : "Failed");
        } else {
            respond("Failed, could not verify client");
        }
        break;
    }
    case AdminRequest::Type::CHECK_LOGGING_FILE:
    {
        std::set<std::string> list;
        std::set<std::string> loggerIds;
        if (!request.loggerId().empty()) {
            if (!request.clientId().empty() && !registry()->configuration()->isKnownLoggerForClient(request.clientId(), request.loggerId())) {
                respond("Logger not mapped to client");
                return;
            }
            loggerIds.insert(request.loggerId());
        } else if (!request.clientId().empty()) {
            if (registry()->configuration()->knownClientsLoggers().find(request.clientId()) !=
                    registry()->configuration()->knownClientsLoggers().end()) {
                auto setOfLoggerIds = registry()->configuration()->knownClientsLoggers().at(request.clientId());
                for (std::string loggerId : setOfLoggerIds) {
                    loggerIds.insert(loggerId);
                }
            } else {
                respond("No logger mapped for the client");
                return;
            }
        }
        for (std::string loggerId : loggerIds) {
            for (std::string levelStr : request.loggingLevels()) {
                el::Level level = el::LevelHelper::convertFromString(levelStr.c_str());
                if (level == el::Level::Unknown) {
                    respond("Unknown level [" + levelStr + "]");
                    return;
                }
                std::string file = getFile(loggerId, levelStr);
                if (!file.empty()) {
                    list.insert(file);
                }
            }
        }

        respond(JsonObject::Json(list).dump());
        break;
    }
    case AdminRequest::Type::FORCE_LOG_ROTATION:
    {
        if (el::Loggers::getLogger(request.loggerId(), false) == nullptr) {
            respond("Logger [" + request.loggerId() + "] not yet registered");
            return;
        }
        if (!m_registry->logRotator()->isRunning()) {
            m_registry->logRotator()->rotate(request.loggerId());
            m_registry->logRotator()->archiveRotatedItems();
            respond("Logs for logger [" + request.loggerId() + "] rotated");
        } else {
            respond("Log rotator already running. Please try later.");
        }
        break;
    }
    default:
        RVLOG(RV_INFO) << "Invalid admin request type";
    }
#endif
}

void AdminRequestHandler::respond(const std::string& response) const
{
    m_session->write(response.c_str(), configuration()->serverKey().c_str());
}