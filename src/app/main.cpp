#include "GUI/mainwindow.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <QProcess>
#include "hardwareinfo.h"
#include "GUI/ewidgetsimpl.h"

void setDefaultFormat() {
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(0);
    //format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    //format.setSwapInterval(0); // Disable vertical refresh syncing
    QSurfaceFormat::setDefaultFormat(format);
}

int main(int argc, char *argv[]) {
    setDefaultFormat();
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication a(argc, argv);
    //#ifdef QT_DEBUG
    //    const qint64 pId = QCoreApplication::applicationPid();
    //    QProcess * const process = new QProcess(&w);
    //    process->start("prlimit --data=3000000000 --pid " + QString::number(pId));
    //#endif

    HardwareInfo::sUpdateInfo();
    eWidgetsImpl widImpl;

    MainWindow w;
    w.show();

    try {
        return a.exec();
    } catch(const std::exception& e) {
        gPrintExceptionFatal(e);
        return -1;
    }
}
