#ifndef SAMPLE_LOG_EX
#define SAMPLE_LOG_EX

#include <residue/extensions.h>

using namespace residue;

class SimpleLogExtension : public LogExtension {
public:
	SimpleLogExtension() : LogExtension("simple-log-extension") 
	{}
	
	virtual ~SimpleLogExtension() = default;

	virtual Extension::Result execute(const LogExtension::Data* const) override;
};

#endif


