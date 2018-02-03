//
//  known-logger-configurator.h
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

#ifndef KnownLoggerConfigurator_h
#define KnownLoggerConfigurator_h

#include "src/logging/log.h"
#include "src/non-copyable.h"
#include "src/utils/utils.h"

namespace residue {

class Configuration;
class UserLogBuilder;

///
/// \brief Configures the known loggers with their respective configuration file.
/// Known loggers are the loggers specified under 'known_loggers' in residue configurations
///
class KnownLoggerConfigurator final : public el::LoggerRegistrationCallback, NonCopyable
{
public:
    explicit KnownLoggerConfigurator() : m_configuration(nullptr)
    {
    }

    inline void setConfiguration(const Configuration* configuration)
    {
        m_configuration = configuration;
    }

    inline void setUserLogBuilder(const UserLogBuilder* userLogBuilder)
    {
        m_userLogBuilder = userLogBuilder;
    }
protected:

    virtual void handle(const el::Logger* logger)
    {
        KnownLoggerConfigurator* f = static_cast<KnownLoggerConfigurator*>(this);
        std::string configFile = f->m_configuration->getConfigurationFile(logger->id(), m_userLogBuilder);

        if (!configFile.empty()) {
            el::Configurations confs(configFile);

            el::base::type::EnumType lIndex = el::LevelHelper::kMinValid;
            el::Loggers::reconfigureLogger(const_cast<el::Logger*>(logger), confs);
            std::vector<std::string> doneList;
            el::LevelHelper::forEachLevel(&lIndex, [&](void) -> bool {
                el::Configuration* filenameConf = confs.get(el::LevelHelper::castFromInt(lIndex), el::ConfigurationType::Filename);
                if (filenameConf != nullptr && std::find(doneList.begin(), doneList.end(), filenameConf->value()) == doneList.end()) {
                    doneList.push_back(filenameConf->value());
                    Utils::updateFilePermissions(filenameConf->value().data(), logger, m_configuration);
                }
                return false; // don't exit yet
            });
        }
    }
private:
    const Configuration* m_configuration;
    const UserLogBuilder* m_userLogBuilder;
};
}
#endif /* KnownLoggerConfigurator_h */
