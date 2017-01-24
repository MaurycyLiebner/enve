#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    QFile file("/home/ailuropoda/.Qt_pro/AniVect/stylesheet.qss");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        w.setStyleSheet(file.readAll());
        file.close();
    }

    w.show();

    return a.exec();
}
