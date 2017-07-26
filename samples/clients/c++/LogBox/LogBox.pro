#-------------------------------------------------
#
# Project created by QtCreator 2017-03-02T12:57:27
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LogBox
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_CXXFLAGS += -std=c++11

DEFINES += ELPP_FEATURE_ALL \ # Optional: We use performance tracking here
    ELPP_MULTI_LOGGER_SUPPORT \
    ELPP_THREAD_SAFE \ # This is absolutely required
    ELPP_QT_LOGGING # We also log qt classes

SOURCES += main.cpp\
        logbox.cpp

HEADERS  += logbox.h \
    log.h

FORMS    += logbox.ui

LIBS += -L"/usr/local/lib" -lresidue

## Following has nothing to do with residue, it's just setup issue with Qt
## http://stackoverflow.com/questions/38131011/qt-application-throws-dyld-symbol-not-found-cg-jpeg-resync-to-restart
LIBS += -L"/System/Library/Frameworks/ImageIO.framework/Versions/A/Resources/"

INCLUDEPATH += /usr/local/include/
