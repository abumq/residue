//
//  command-handler.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef CommandHandler_h
#define CommandHandler_h

#include <functional>
#include <string>
#include <set>
#include <vector>
#include <unordered_map>
#include "src/non-copyable.h"
#include "src/plugins/plugin.h"

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

    inline void registerCommand(std::string&& cmd,
                                CommandHandlerFunction&& action)
    {
        m_commands.insert(std::make_pair(std::move(cmd), std::move(action)));
    }

    inline void registerPlugin(std::unique_ptr<Plugin>&& plugin)
    {
        m_plugins.insert(std::move(plugin));
    }

    const Plugin* findById(const std::string& id) const;

    void takeInput(bool *exitOnInterrupt);

    bool m_running;
    std::set<std::string> m_history;
    std::set<std::unique_ptr<Plugin>> m_plugins;
    std::unordered_map<std::string, CommandHandlerFunction> m_commands;
};
}

#endif /* CommandHandler_h */
