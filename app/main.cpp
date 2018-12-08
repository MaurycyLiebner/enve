#include "GUI/mainwindow.h"
#include "application.h"
#include <QSurfaceFormat>
#include <QProcess>

void setDefaultFormat() {
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOptions(QSurfaceFormat::DeprecatedFunctions);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setSwapInterval(0); // Disable vertical refresh syncing
    QSurfaceFormat::setDefaultFormat(format);
}

int main(int argc, char *argv[]) {
    Application a(argc, argv);
    setDefaultFormat();

    MainWindow w;
//#ifdef QT_DEBUG
//    qint64 pId = QCoreApplication::applicationPid();
//    QProcess *process = new QProcess(&w);
//    process->start("prlimit --data=1500000000 --pid " + QString::number(pId));
//#endif

    w.show();

    return a.exec();
}
