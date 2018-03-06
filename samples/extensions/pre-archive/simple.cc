#include "simple.h"
#include <iostream>

RESIDUE_EXTENSION(SimplePre, "1.0.0")

residue::Extension::Result SimplePre::process(void* d)
{
    PreArchiveExtension::Data* data = static_cast<PreArchiveExtension::Data*>(d);
    if (!data) {
        std::cout << "SimplePre::process() No data found " << std::endl;
        return {0, true};
    }
	std::cout << "About to archive for logger: " << data->loggerId << "- number of files: " << data->files.size() << " to file " << data->archiveFilename << std::endl;
	return {0, true};
}
