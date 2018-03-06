#ifndef SAMPLE_PRE_ARCHIVE_EX
#define SAMPLE_PRE_ARCHIVE_EX

#include "../../../src/extensions/pre-archive-extension.h"

class SimplePre : public residue::PreArchiveExtension {
public:
	SimplePre() : PreArchiveExtension("simple-pre") {}
	virtual ~SimplePre() = default;

	virtual residue::Extension::Result process(void*) override;
};

#endif


