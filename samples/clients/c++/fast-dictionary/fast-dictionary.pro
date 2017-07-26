QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fast-dictionary
TEMPLATE = app

COMPILER = g++
QMAKE_CC = $$COMPILER
QMAKE_CXX = $$COMPILER
QMAKE_LINK = $$COMPILER

QMAKE_CXXFLAGS += -std=c++11

DEFINES += ELPP_FEATURE_ALL \
    ELPP_MULTI_LOGGER_SUPPORT \
    ELPP_THREAD_SAFE

SOURCES += main.cc\
        mainwindow.cc \
    listwithsearch.cc \

HEADERS  += mainwindow.hh \
    listwithsearch.hh \
    log.h

FORMS    += mainwindow.ui

LIBS += -L"/usr/local/lib" -lresidue


## Following has nothing to do with residue, it's just setup issue with Qt
## http://stackoverflow.com/questions/38131011/qt-application-throws-dyld-symbol-not-found-cg-jpeg-resync-to-restart
LIBS += -L"/System/Library/Frameworks/ImageIO.framework/Versions/A/Resources/"

INCLUDEPATH += /usr/local/include/


DISTFILES += \
    words.txt
