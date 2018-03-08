#include "log-analyzer.h"
#include <iostream>

RESIDUE_EXTENSION(LogAnalyzer, "1.0.0")

Extension::Result LogAnalyzer::execute(const LogExtension::Data* const data)
{
    if (!data) {
        writeLog("Data not passed in", Extension::LogLevel::Error);
        return {1, true};
    }
    switch ()
    writeLog("SimpleLogExtension->config->key: " + conf().get<std::string>("key", ""), Extension::LogLevel::Debug);
    std::cout << "SimpleLogExtension::execute() ==>" << data->formattedMessage << std::endl;

    return {0, true};
}
