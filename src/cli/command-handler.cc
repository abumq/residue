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

#ifdef RESIDUE_USE_READLINE_LIB
#   include <cstdio>
#   include <readline/readline.h>
#   include <readline/history.h>
#endif // RESIDUE_USE_READLINE_LIB
#include "src/logging/log.h"
#include "src/cli/command-handler.h"
#include "src/utils/utils.h"
#include "src/cli/update.h"
#include "src/cli/reset.h"
#include "src/cli/reload-config.h"
#include "src/cli/session-details.h"
#include "src/cli/stats.h"
#include "src/cli/rotate.h"
#include "src/cli/list-tokens.h"
#include "src/cli/list-logging-files.h"
#include "src/cli/clients.h"

using namespace residue;

const std::size_t CommandHandler::MAX_COMMANDS_HISTORY = 5;

CommandHandler::CommandHandler(Registry* registry) :
    m_running(false)
{

    registerRawCommand("quit", [&, this](std::vector<std::string>&&, std::ostringstream&, bool) {
        m_running = false;
    });

    registerRawCommand("exit", [&, this](std::vector<std::string>&&, std::ostringstream&, bool) {
        m_running = false;
    });

    registerRawCommand("history", [&, this](std::vector<std::string>&&, std::ostringstream& result, bool) {
        int i = 1;
        for (auto& c : m_history) {
            result << (i++) << " > " << c << std::endl;
        }
    });

    registerRawCommand("help", [&, this](std::vector<std::string>&& params, std::ostringstream& result, bool) {
        if (!params.empty()) {
            const Command* ext = findByName(params.at(0));
            if (ext != nullptr) {
                std::string s(ext->name());
                result << Utils::toLower(s) << ":\n    "
                       << ext->description()
                       << "\nUsage:\n" << ext->help()
                       << std::endl;
            }
        } else {
            result << "Commands:" << std::endl;
            for (const auto& c : m_rawCommands) {
                if (c.first != "help") {
                    std::string s(c.first);
                    result << "    " << Utils::toLower(s) << std::endl;
                }
            }
            for (const auto& c : m_commands) {
                std::string s(c->name());
                result << "    " << Utils::toLower(s) << std::endl;
            }
        }
    });

    registerCommand(std::unique_ptr<Command>(new Update(registry)));
    registerCommand(std::unique_ptr<Command>(new Reset(registry)));
    registerCommand(std::unique_ptr<Command>(new ReloadConfig(registry)));
    registerCommand(std::unique_ptr<Command>(new SessionDetails(registry)));
    registerCommand(std::unique_ptr<Command>(new Stats(registry)));
    registerCommand(std::unique_ptr<Command>(new Rotate(registry)));
    registerCommand(std::unique_ptr<Command>(new ListTokens(registry)));
    registerCommand(std::unique_ptr<Command>(new ListLoggingFiles(registry)));
    registerCommand(std::unique_ptr<Command>(new Clients(registry)));
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

    const Command* command = findByName(cmd);

    if (command != nullptr) {
        if (cmd != "history") {
            m_history.insert(cmd);
        }
        command->execute(std::move(params), result, ignoreConfirmation);
    } else if (m_rawCommands.find(cmd) != m_rawCommands.end()) {
        if (cmd != "history") {
            m_history.insert(cmd);
        }
        m_rawCommands.at(cmd)(std::move(params), result, ignoreConfirmation);
    }
}

const Command* CommandHandler::findByName(const std::string& name) const
{
    auto p = std::find_if(m_commands.begin(), m_commands.end(), [&](const std::unique_ptr<Command>& e) {
        return e->name() == name;
    });
    if (p != m_commands.end()) {
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
#ifdef RESIDUE_USE_READLINE_LIB
    char* line = readline("Residue > ");
    if (line != nullptr) {
        add_history(line);
        input = std::string(line);
        free(line);
    }
#else
    std::cout << std::endl << "Residue > ";
    std::getline(std::cin, input);
#endif
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
