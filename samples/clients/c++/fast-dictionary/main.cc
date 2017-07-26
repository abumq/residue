#include "mainwindow.hh"
#include <QApplication>
#include "log.h"

TIMED_SCOPE(app, "app");

int main(int argc, char *argv[])
{

    // Optional and recommended for sending verbose logs
    Residue::setApplicationArgs(argc, argv);

    Residue::AccessCodeMap accessCodes = {
        {
            "sample-app", "a2dcb"
        }
    };

    Residue::connect(Residue::LOCALHOST, Residue::DEFAULT_PORT, &accessCodes);

    // Following 2 lines are optional for information. We will display them using
    // %app and %thread_name format specifiers

    Residue::setApplicationId("com.muflihun.residue.samples.fast-dictionary");
    Residue::setThreadName("MainThread");

    QApplication a(argc, argv);
    MainWindow w(argc, argv);
    w.show();

    int st = a.exec();

    Residue::disconnect();
    return st;
}
