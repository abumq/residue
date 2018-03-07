#include "simple.h"
#include <iostream>

RESIDUE_EXTENSION(SimplePost, "1.0.0")

Extension::Result SimplePost::execute(const PostArchiveExtension::Data* const data)
{
    if (!data) {
        writeLog("Data not passed in", Extension::LogLevel::Error);
        return {1, true};
    }
	std::cout << "Created new file: " 
                << data->archiveFilename << " - logger: " 
                << data->loggerId << std::endl;
	return {0, true};
}
