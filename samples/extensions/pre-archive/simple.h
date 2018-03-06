#ifndef SAMPLE_PRE_ARCHIVE_EX
#define SAMPLE_PRE_ARCHIVE_EX

#include <residue/extensions.h>

class SimplePre : public residue::PreArchiveExtension {
public:
	SimplePre() : PreArchiveExtension("simple-pre") {}
	virtual ~SimplePre() = default;

	virtual residue::Extension::Result execute(void*) override;
};

#endif


