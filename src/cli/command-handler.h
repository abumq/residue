//
//  command-handler.h
//  Residue
//
//  Copyright 2017-present @abumq (Majid Q.)
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

#ifndef CommandHandler_h
#define CommandHandler_h

#include <functional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "cli/command.h"
#include "non-copyable.h"

namespace residue {

class Registry;

///
/// \brief Handles user commands
///
class CommandHandler final : NonCopyable
{
public:
    using CommandHandlerFunction = std::function<void(std::vector<std::string>&&, std::ostringstream&, bool)>;

    static const std::size_t MAX_COMMANDS_HISTORY;

    explicit CommandHandler(Registry* registry);

    void handle(std::string&&,
                std::vector<std::string>&&,
                std::ostringstream& result,
                bool ignoreConfirmation = false);

    inline bool running() const
    {
        return m_running;
    }

    void start(bool *exitOnInterrupt);

private:

    inline void registerRawCommand(std::string&& name,
                                   CommandHandlerFunction&& action)
    {
        m_rawCommands.insert(std::make_pair(std::move(name), std::move(action)));
    }

    inline void registerCommand(std::unique_ptr<Command>&& command)
    {
        m_commands.insert(std::move(command));
    }

    const Command* findByName(const std::string& name) const;

    void takeInput(bool *exitOnInterrupt);

    bool m_running;
    std::set<std::string> m_history;
    std::set<std::unique_ptr<Command>> m_commands;
    std::unordered_map<std::string, CommandHandlerFunction> m_rawCommands;
};
}

#endif /* CommandHandler_h */
