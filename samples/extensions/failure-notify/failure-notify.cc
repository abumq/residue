#include "failure-notify.h"

#include <cstring>

#include <iostream>
#include <sstream>

RESIDUE_EXTENSION(FailureNotify, "1.0.0")

Extension::Result FailureNotify::execute(const DispatchErrorExtension::Data* const data)
{
    if (!data) {
        writeLog("Data not passed in", Extension::LogLevel::Error);
        return {1, true};
    }

    m_failureCount++;

    if (m_failureCount >= conf().get<unsigned long>("threshold", 200UL)) {
        notifyRecipients(data);
        m_failureCount = 0UL;
    }

    return {0, true};
}

void FailureNotify::notifyRecipients(const DispatchErrorExtension::Data* const data)
{
    JsonDoc recipients(conf().getArr("recipients");
    if (recipients.isArray()) {
        std::stringstream ss;
        for (const auto& recipientNode : recipients) {
            JsonDoc recipient(recipientNode);
            std::string email = recipient.as<std::string>("");
            writeLog("Notifying " + email);
            
            ss << "echo 'Residue dispatch error " 
               << std::strerror(data->errorNumber) << ". File: " 
               << data->filename << "' | mail -s 'residue err' " << email;
            system(ss.str().c_str());
        }
    }
}
