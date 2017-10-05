//
//  main.cc
//  Residue Tests
//
//  Copyright © 2017 Muflihun Labs
//

#include "test.h"
#include "configuration-test.h"
#include "utils-test.h"
#include "license-manager-test.h"
#include "crypto-test.h"
#include "url-test.h"
#include "admin-request-test.h"
#include "task-schedule-test.h"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    std::cout << "Default log file: " << ELPP_DEFAULT_LOG_FILE << std::endl;

    el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);

    return ::testing::UnitTest::GetInstance()->Run();
}
