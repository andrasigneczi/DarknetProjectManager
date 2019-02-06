QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DarknetProjectManager
TEMPLATE = lib


SOURCES += src/project.cpp src/util.cpp src/mainwindow.cpp src/labelingcanvas.cpp

HEADERS  += src/project.h src/util.h src/mainwindow.h src/labelingcanvas.h

#FORMS    += mainwindow.ui

OBJECTS_DIR=obj

win32 {

#INCLUDEPATH += d:\armadillo-8.500.0\include
SOURCES += src/main.cpp
#LIBS += d:/armadillo-8.500.0/Release/armadillo.lib d:/armadillo-8.500.0/examples/lib_win64/blas_win64_MT.lib
#LIBS += d:/armadillo-8.500.0/examples/lib_win64/lapack_win64_MT.lib
#LIBS += c:/Users/Andras/IdeaProjects/TravelOptimizer/AI-machine_learning/build-MachineLearning-Desktop_Qt_5_10_1_MSVC2017_64bit-Release/release/MachineLearning.lib
} else {
QMAKE_CXXFLAGS += -std=c++14 -O3 -Wall -Wextra -pedantic -D__LINUX__  -pthread -fPIC -march=native \ 
                  -I/usr/include/qt5/QtWidgets -I/usr/include/qt5/QtCore -I/usr/include/qt5/QtWidgets -I/usr/include/qt5/QtGui \
                  -I/usr/include/qt5/QtPrintSupport -I/usr/include/qt5

QMAKE_CXX = g++

#unix:!macx: LIBS += -L$$PWD/src/ -lQt5PrintSupport
}

INCLUDEPATH += $$PWD/src
DEPENDPATH += $$PWD/src
