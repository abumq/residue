#ifndef LOG_ANALYZER_H
#define LOG_ANALYZER_H

#include <residue/extensions.h>

using namespace residue;

class LogAnalyzer : public LogExtension {
public:
	LogAnalyzer() : LogExtension("log-analyzer") 
	{}
	
	virtual ~LogAnalyzer() = default;

	virtual Extension::Result execute(const LogExtension::Data* const) override;
};

#endif


