#ifndef SAMPLE_POST_ARCHIVE_EX
#define SAMPLE_POST_ARCHIVE_EX

#include <residue/extensions.h>

class SimplePost : public residue::PostArchiveExtension {
public:
	SimplePost() : PostArchiveExtension("simple-post") {}
	virtual ~SimplePost() = default;

	virtual residue::Extension::Result execute(void*) override;
};

#endif


