//
//  command-handler.cc
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

#include "include/log.h"
#include "src/core/command-handler.h"
#include "src/utils/utils.h"

#include "src/plugins/update.h"
#include "src/plugins/reset.h"
#include "src/plugins/reload-config.h"
#include "src/plugins/session-details.h"
#include "src/plugins/stats.h"
#include "src/plugins/rotate.h"
#include "src/plugins/list-tokens.h"
#include "src/plugins/list-logging-files.h"
#include "src/plugins/clients.h"

using namespace residue;

const std::size_t CommandHandler::MAX_COMMANDS_HISTORY = 5;

CommandHandler::CommandHandler(Registry* registry) :
    m_running(false)
{

    registerCommand("quit", [&, this](std::vector<std::string>&&, std::ostringstream&, bool) {
        m_running = false;
    });

    registerCommand("exit", [&, this](std::vector<std::string>&&, std::ostringstream&, bool) {
        m_running = false;
    });

    registerCommand("history", [&, this](std::vector<std::string>&&, std::ostringstream& result, bool) {
        int i = 1;
        for (auto& c : m_history) {
            result << (i++) << " > " << c << std::endl;
        }
    });

    registerCommand("help", [&, this](std::vector<std::string>&& params, std::ostringstream& result, bool) {
        if (!params.empty()) {
            const Plugin* ext = findById(params.at(0));
            if (ext != nullptr) {
                std::string s(ext->id());
                result << Utils::toLower(s) << ":\n    "
                       << ext->description()
                       << "\nUsage:\n" << ext->help()
                       << std::endl;
            }
        } else {
            result << "Commands:" << std::endl;
            for (const auto& c : m_commands) {
                if (c.first != "help") {
                    std::string s(c.first);
                    result << "    " << Utils::toLower(s) << std::endl;
                }
            }
            for (const auto& c : m_plugins) {
                std::string s(c->id());
                result << "    " << Utils::toLower(s) << std::endl;
            }
        }
    });

    registerPlugin(std::unique_ptr<Plugin>(new Update(registry)));
    registerPlugin(std::unique_ptr<Plugin>(new Reset(registry)));
    registerPlugin(std::unique_ptr<Plugin>(new ReloadConfig(registry)));
    registerPlugin(std::unique_ptr<Plugin>(new SessionDetails(registry)));
    registerPlugin(std::unique_ptr<Plugin>(new Stats(registry)));
    registerPlugin(std::unique_ptr<Plugin>(new Rotate(registry)));
    registerPlugin(std::unique_ptr<Plugin>(new ListTokens(registry)));
    registerPlugin(std::unique_ptr<Plugin>(new ListLoggingFiles(registry)));
    registerPlugin(std::unique_ptr<Plugin>(new Clients(registry)));
}

void CommandHandler::handle(std::string&& cmd,
                            std::vector<std::string>&& params,
                            std::ostringstream& result,
                            bool ignoreConfirmation)
{
    Utils::toLower(cmd);
    Utils::trim(cmd);

    while (m_history.size() > MAX_COMMANDS_HISTORY) {
        m_history.erase(m_history.begin());
    }

    const Plugin* plugin = findById(cmd);

    if (plugin != nullptr) {
        if (cmd != "history") {
            m_history.insert(cmd);
        }
        plugin->execute(std::move(params), result, ignoreConfirmation);
    } else if (m_commands.find(cmd) != m_commands.end()) {
        if (cmd != "history") {
            m_history.insert(cmd);
        }
        m_commands.at(cmd)(std::move(params), result, ignoreConfirmation);
    }
}

const Plugin* CommandHandler::findById(const std::string& id) const
{
    auto p = std::find_if(m_plugins.begin(), m_plugins.end(), [&](const std::unique_ptr<Plugin>& e) {
        return e->id() == id;
    });
    if (p != m_plugins.end()) {
        return p->get();
    }
    return nullptr;
}

void CommandHandler::start(bool* exitOnInterrupt)
{
    m_running = true;
    takeInput(exitOnInterrupt);
}

void CommandHandler::takeInput(bool* exitOnInterrupt)
{
    std::string input;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::cout << std::endl << "Residue > ";
    std::getline(std::cin, input);
    if (!input.empty()) {
        *exitOnInterrupt = false; // reset as user entered a command
        std::vector<std::string> params;
        std::string cmd;
        std::size_t pos = input.find(" ");
        if (pos != std::string::npos) {
            cmd = input.substr(0, pos);
            std::size_t lastPos = pos;
            while ((pos = input.find(" ", lastPos + 1)) != std::string::npos) {
                params.push_back(input.substr(lastPos + 1, pos - lastPos - 1));
                lastPos = pos;
            }
            params.push_back(input.substr(lastPos + 1));
        } else {
            cmd = std::move(input);
        }
        std::ostringstream result;
        handle(std::move(cmd), std::move(params), result);
        std::cout << result.str();
    }
    if (m_running) {
        takeInput(exitOnInterrupt);
    }
}
