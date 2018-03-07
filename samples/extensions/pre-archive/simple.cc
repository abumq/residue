#include "simple.h"
#include <iostream>

RESIDUE_EXTENSION(SimplePre, "1.0.0")

using namespace residue;

Extension::Result SimplePre::execute(void* d)
{
    PreArchiveExtension::Data* data = static_cast<PreArchiveExtension::Data*>(d);
    if (!data) {
        writeLog("Data not passed in", Extension::LogLevel::Error);
        return {1, true};
    }
	std::cout << "About to archive for logger: " << data->loggerId 
                << "- number of files: " << data->files.size() 
                << " to file " << data->archiveFilename << std::endl;
	return {0, true};
}
