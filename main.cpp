#include "mainwindow.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);    
    QCoreApplication::setOrganizationName("freedom");
    QCoreApplication::setApplicationName("similar_vision");

    MainWindow w;
    w.show();

    return a.exec();
}
