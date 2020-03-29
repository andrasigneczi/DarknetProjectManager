QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DarknetProjectManagerBin
TEMPLATE = app

SOURCES += ../tests/main.cpp

OBJECTS_DIR=obj

INCLUDEPATH += $$PWD/../src
#INCLUDEPATH += $$PWD/../../../Util/src
#INCLUDEPATH += $$PWD/../../../Backend/src
#INCLUDEPATH += $$PWD/../../../HQ/src
#DEPENDPATH += $$PWD/../src

#LIBS += -L"$$PWD/../../../Util/obj/debug" -lbotutil
#LIBS += -L"$$PWD/../../../HQ/obj/debug" -lHQ
LIBS += -L"$$PWD/../debug/debug" -lDarknetProjectManager
