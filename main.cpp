#include "mainwindow.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <QProcess>

void setDefaultFormat()
{
    QSurfaceFormat format;
#ifdef Q_OS_OSX
    format.setVersion(3, 2);
    format.setProfile(QSurfaceFormat::CoreProfile);
#else
    // XXX This can be removed once we move to Qt5.7
    format.setVersion(3, 0);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOptions(QSurfaceFormat::DeprecatedFunctions);
#endif
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setSwapInterval(0); // Disable vertical refresh syncing
    QSurfaceFormat::setDefaultFormat(format);
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    setDefaultFormat();
//    QSurfaceFormat  format;

//    //format.setVersion(3,2);
//    format.setSamples(0);
//    format.setStencilBufferSize(8);
//    //format.setRenderableType(QSurfaceFormat::OpenGLES);
//    //format.setSwapBehavior(QSurfaceFormat::SingleBuffer);

//    //format.setProfile(QSurfaceFormat::CompatibilityProfile);
//    QSurfaceFormat::setDefaultFormat(format);

    MainWindow w;
#ifdef QT_DEBUG
    qint64 pId = QCoreApplication::applicationPid();
    QProcess *process = new QProcess(&w);
    process->start("prlimit --data=1500000000 --pid " + QString::number(pId));
#endif

    w.show();

    return a.exec();
}
