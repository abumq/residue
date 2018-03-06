#include "simple.h"
#include <iostream>

RESIDUE_EXTENSION(Simple, "1.0.0")

residue::Extension::Result Simple::execute(void* d)
{
    LogExtension::Data* data = static_cast<LogExtension::Data*>(d);
    if (!data) {
        std::cout << "Simple::execute() No data found " << std::endl;
        return {0, true};
    }
    std::cout << "Simple extension key: " << conf().get<std::string>("key", "") << std::endl;
	std::cout << "Simple::execute() ==>" << data->formattedMessage << std::endl;
	return {0, true};
}
