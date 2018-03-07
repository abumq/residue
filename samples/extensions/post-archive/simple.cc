#include "simple.h"
#include <iostream>

RESIDUE_EXTENSION(SimplePost, "1.0.0")

Extension::Result SimplePost::execute(void* d)
{
    PostArchiveExtension::Data* data = static_cast<PostArchiveExtension::Data*>(d);
    if (!data) {
        writeLog("Data not passed in", Extension::LogLevel::Error);
        return {1, true};
    }
	std::cout << "Created new file: " 
                << data->archiveFilename << " - logger: " 
                << data->loggerId << std::endl;
	return {0, true};
}
