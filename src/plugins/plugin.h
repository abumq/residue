//
//  plugin.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef Plugin_h
#define Plugin_h

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
#include "include/log.h"

namespace residue {

class Registry;

///
/// \brief Base plugin interface
///
class Plugin
{
public:
    Plugin(const std::string& id, const std::string& description, const std::string& help, Registry* registry);

    inline std::string id() const
    {
        return m_id;
    }

    inline std::string description() const
    {
        return m_description;
    }

    inline std::string help() const
    {
        return m_help;
    }

    inline Registry* registry() const
    {
        return m_registry;
    }

    bool getConfirmation(const std::string& help) const;

    inline bool hasParam(const std::vector<std::string>& params, const std::string& param) const
    {
        return std::find(params.begin(), params.end(), param) != params.end();
    }

    ///
    /// \brief getParamValue Get param value e.g, --client_id blah will return blah
    ///
    std::string getParamValue(const std::vector<std::string>& params, const std::string& param) const;

    virtual void execute(std::vector<std::string>&&, std::ostringstream&, bool) const = 0;

private:
    std::string m_id;
    std::string m_description;
    std::string m_help;
    Registry* m_registry;
};
}

#endif /* Plugin_h */