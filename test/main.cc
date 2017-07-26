//
//  main.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include "test.h"
#include "ConfigurationTest.h"
#include "UtilsTest.h"
#include "UrlTest.h"
#include "AdminRequestTest.h"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);

    return ::testing::UnitTest::GetInstance()->Run();
}
