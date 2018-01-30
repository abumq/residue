//
//  extension.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include "src/plugins/plugin.h"
#include "src/core/registry.h"

using namespace residue;

Plugin::Plugin(const std::string& id, const std::string& description, const std::string& help, Registry* registry) :
    m_id(id),
    m_description(description),
    m_help(help),
    m_registry(registry)
{
}

bool Plugin::getConfirmation(const std::string& help) const
{
    std::cout << "Are you sure? (Y/N" << (help.empty() ? "" : "/?") << "): ";
    std::string confirm;
    std::getline(std::cin, confirm);
    if (Utils::toLower(confirm) == "y" || Utils::toLower(confirm) == "yes") {
        return true;
    } else if (confirm == "?") {
        std::cout << help << "\n";
    }
    std::cout << std::endl;
    return false;
}

std::string Plugin::getParamValue(const std::vector<std::string>& params, const std::string& param) const
{
    auto pos = std::find(params.begin(), params.end(), param);
    if (pos != params.end()) {
        if (pos < params.end() - 1) {
            auto nx = std::next(pos);
            return *nx;
        }
    }
    return "";
}