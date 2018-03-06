#include "simple.h"
#include <iostream>

RESIDUE_EXTENSION(Simple, "1.0.0")

bool Simple::process(void* d)
{
        LogExtension::Data* data = static_cast<LogExtension::Data*>(d);
        if (!data) {
            std::cout << "Simple::process() No data found " << std::endl;
            return true;
        }
	std::cout << "Simple::process() ->" << data->formattedMessage << std::endl;
	return true;
}
