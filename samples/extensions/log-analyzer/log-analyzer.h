#ifndef LOG_ANALYZER_H
#define LOG_ANALYZER_H

#include <residue/extensions.h>

using namespace residue;

class LogAnalyzer : public LogExtension {
public:
	static const char kFormatSpecifier;
	static const std::string kMsgFormatSpecifier;
	static const std::string kFormattedMsgFormatSpecifier;

	LogAnalyzer() : LogExtension("log-analyzer") 
	{}
	
	virtual ~LogAnalyzer() = default;

	virtual Extension::Result execute(const LogExtension::Data* const) override;
private:
	void resolveFormatSpecifier(std::string& subject, const std::string& formatSpecifer, const std::string& replace);
};

#endif


