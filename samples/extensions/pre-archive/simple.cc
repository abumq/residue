#include "simple.h"
#include <iostream>

RESIDUE_EXTENSION(SimplePre, "1.0.0")

using namespace residue;

Extension::Result SimplePre::execute(const PreArchiveExtension::Data* const data)
{
    if (!data) {
        writeLog("Data not passed in", Extension::LogLevel::Error);
        return {1, true};
    }
	std::cout << "About to archive for logger: " << data->loggerId 
                << "- number of files: " << data->files.size() 
                << " to file " << data->archiveFilename << std::endl;
    if (data->files.size() > 0) {
        for (auto f : data->files) {
            std::cout << f.first << " => " << f.second << std::endl;
        }
    }
	return {0, true};
}
