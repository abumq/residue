#include "simple.h"
#include <iostream>

RESIDUE_EXTENSION(SimplePost, "1.0.0")

residue::Extension::Result SimplePost::process(void* d)
{
        PostArchiveExtension::Data* data = static_cast<PostArchiveExtension::Data*>(d);
        if (!data) {
            std::cout << "Simple::process() No data found " << std::endl;
            return {0, true};
        }
	std::cout << "Created new file: " << data->archiveFilename << " - logger: " << data->loggerId << std::endl;
	return {0, true};
}
