#include <iostream>
#include <QtWidgets/QApplication>
#include <QtCore/QSettings>
#include <QtCore/QDebug>
#include "project.h"
#include <QDir>
#include "mainwindow.h"

int main (int argc, char* argv[]) {
    //QCoreApplication app(argc, argv);
    QApplication app(argc, argv);

    //string newPath("/home/andrej/src/EXTERNAL/darknet/snowman/YOLOv3-Training-Snowman-Detector");
    //string newPath(".");
    //string newPath("/home/andrej/src/EXTERNAL/darknet_cuda/coc");
    //string newPath("/home/andrej/src/DarkCOCBot/Backend/tests/darknet_cfg");
    string newPath("darknet_cfg");
    QString path = QDir::currentPath();    
    if(QDir::setCurrent(newPath.c_str())) {
        
        try {
            Project project;
            project.openProject("darknet.data");
            //project.openProject("test.data");
            MainWindow w(project);
            w.show();
            app.exec();
        } catch(string str) {
            cerr << str << endl;
        }
        QDir::setCurrent(path);
    } else {
        cerr << "Cannot find the path: " << newPath << endl;
    }
}
