#ifndef SAMPLE_PRE_ARCHIVE_EX
#define SAMPLE_PRE_ARCHIVE_EX

#include <residue/extensions.h>

using namespace residue;

class SimplePre : public PreArchiveExtension {
public:
	SimplePre() : PreArchiveExtension("simple-pre") {}
	virtual ~SimplePre() = default;

	virtual Extension::Result execute(const PreArchiveExtension::Data* const) override;
};

#endif


