QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DarknetProjectManager
TEMPLATE = lib
win32:CONFIG += staticlib


SOURCES += src/project.cpp src/util.cpp src/mainwindow.cpp src/labelingcanvas.cpp

HEADERS  += src/project.h src/util.h src/mainwindow.h src/labelingcanvas.h

#FORMS    += mainwindow.ui

OBJECTS_DIR=obj

win32 {
#SOURCES += src/main.cpp
} else {
QMAKE_CXXFLAGS += -std=c++17 -O3 -Wall -Wextra -pedantic -D__LINUX__  -pthread -fPIC -march=native \ 
                  -I/usr/include/qt5/QtWidgets -I/usr/include/qt5/QtCore -I/usr/include/qt5/QtWidgets -I/usr/include/qt5/QtGui \
                  -I/usr/include/qt5/QtPrintSupport -I/usr/include/qt5

QMAKE_CXX = g++
}

INCLUDEPATH += $$PWD/src
DEPENDPATH += $$PWD/src
