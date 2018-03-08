#include "log-analyzer.h"
#include <iostream>

RESIDUE_EXTENSION(LogAnalyzer, "1.0.0")

const char LogAnalyzer::kFormatSpecifier = '%';
const std::string LogAnalyzer::kMsgFormatSpecifier = std::string(1, kFormatSpecifier) + "msg";
const std::string LogAnalyzer::kFormattedMsgFormatSpecifier = std::string(1, kFormatSpecifier) + "formatted_msg";

Extension::Result LogAnalyzer::execute(const LogExtension::Data* const data)
{
    if (!data) {
        writeLog("Data not passed in", Extension::LogLevel::Error);
        return {1, true};
    }
    std::string cmd;
    switch (static_cast<LogExtension::Level>(data->level)) {
    case LogExtension::Level::Info:
        cmd = conf().get<std::string>("info", "");
        break;
    case LogExtension::Level::Error:
        cmd = conf().get<std::string>("error", "");
        break;
    case LogExtension::Level::Warning:
        cmd = conf().get<std::string>("warning", "");
        break;
    case LogExtension::Level::Debug:
        cmd = conf().get<std::string>("debug", "");
        break;
    case LogExtension::Level::Fatal:
        cmd = conf().get<std::string>("fatal", "");
        break;
    case LogExtension::Level::Verbose:
        cmd = conf().get<std::string>("verbose", "");
        break;
    case LogExtension::Level::Trace:
        cmd = conf().get<std::string>("trace", "");
        break;
    }
    if (!cmd.empty()) {
        resolveFormatSpecifier(cmd, kMsgFormatSpecifier, data->message);
        resolveFormatSpecifier(cmd, kFormattedMsgFormatSpecifier, data->formattedMessage);
        system(cmd.c_str());
    }

    return {0, true};
}

void LogAnalyzer::resolveFormatSpecifier(std::string& subject, const std::string& formatSpecifier, const std::string& replace)
{
  std::size_t foundAt = std::string::npos;
  while ((foundAt = subject.find(formatSpecifier, foundAt + 1)) != std::string::npos) {
    if (foundAt > 0 && subject[foundAt - 1] == kFormatSpecifier) {
      subject.erase(foundAt > 0 ? foundAt - 1 : 0, 1);
      ++foundAt;
    } else {
      subject.replace(foundAt, formatSpecifier.length(), replace);
      return;
    }
  }
}