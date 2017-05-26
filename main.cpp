#include "mainwindow.h"
#include <QApplication>
#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSurfaceFormat  format;

    format.setSamples(4);
    format.setStencilBufferSize(8);
    //format.setVersion(3,2);
    //format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    MainWindow w;

    w.show();

    return a.exec();
}
