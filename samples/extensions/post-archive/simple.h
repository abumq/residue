#ifndef SAMPLE_POST_ARCHIVE_EX
#define SAMPLE_POST_ARCHIVE_EX

#include <residue/extensions.h>

using namespace residue;

class SimplePost : public PostArchiveExtension {
public:
	SimplePost() : PostArchiveExtension("simple-post") {}
	virtual ~SimplePost() = default;

	virtual Extension::Result execute(const PostArchiveExtension::Data* const) override;
};

#endif


