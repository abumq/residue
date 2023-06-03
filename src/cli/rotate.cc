//
//  rotate.cc
//  Residue
//
//  Copyright 2017-present @abumq (Majid Q.)
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#include "cli/rotate.h"

#include "core/registry.h"
#include "tasks/log-rotator.h"

using namespace residue;

Rotate::Rotate(Registry* registry) :
    Command("rotate",
            "Trigger log rotation manually for specified logger",
            "rotate --logger-id <id> [--check-only] [--ignore-archive]",
            registry)
{
}

void Rotate::execute(std::vector<std::string>&& params, std::ostringstream& result, bool ignoreConfirmation) const
{
    const std::string loggerId = getParamValue(params, "--logger-id");
    if (loggerId.empty()) {
        result << "No logger ID provided\n";
        return;
    }
    if (el::Loggers::getLogger(loggerId, false) == nullptr) {
        result << "Logger [" << loggerId << "] not yet registered\n";
        return;
    }
    if (hasParam(params, "--check-only")) {
        const auto& frequencies = registry()->configuration()->rotationFreqencies();
        const auto& freqPair = frequencies.find(loggerId);
        if (freqPair != frequencies.end()) {
            if (freqPair->second == Configuration::RotationFrequency::NEVER) {
                result << "Not scheduled\n";
            } else {
                for (const auto& rotator : registry()->logRotators()) {
                    if (rotator->frequency() == freqPair->second) {
                        result << "Logger [" << loggerId << "] uses ["
                               << rotator->name() << "] scheduled for next run @ ["
                               << rotator->formattedNextExecution() << "]\n";
                        break;
                    }
                }
            }
        } else {
            result << "Not scheduled\n";
        }
        return;
    }
    if (ignoreConfirmation || getConfirmation("This ignores the original rotation frequency and forces to run the rotation.\n"
                                              "This will change the rotation schedule for this logger.\n\n"
                                              "--check-only: To only check the next scheduled time\n"
                                              "--ignore-archive: To run the log rotation (if no --check-only provided) but do not archive and compress\n")) {
        const auto& frequencies = registry()->configuration()->rotationFreqencies();
        const auto& freqPair = frequencies.find(loggerId);
        for (auto& rotator : registry()->logRotators()) {
            if (rotator->frequency() != freqPair->second) {
                continue;
            }
            if (rotator->isExecuting()) {
                result << "Log rotator already running, please try later\n";
                return;
            }
            result << "Rotating logs for [" << loggerId << "] using [" << rotator->name() << "]\n";
            rotator->setLastExecution(Utils::now());
            rotator->rotate(loggerId);
            if (!hasParam(params, "--ignore-archive")) {
                result << "Archiving logs for [" << loggerId << "]\n";
                rotator->archiveRotatedItems();
            } else {
                result << "Ignoring archive rotated logs for [" << loggerId << "]\n";
            }

        }
    }
}
