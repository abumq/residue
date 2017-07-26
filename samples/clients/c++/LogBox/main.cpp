#include "logbox.h"
#include <QApplication>

#include "log.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LogBox w;
    w.show();

    return a.exec();
}
