#include "GUI/mainwindow.h"
#include "application.h"
#include <QSurfaceFormat>
#include <QProcess>
#include "Animators/PathAnimators/smartpathcontainer.h"

void setDefaultFormat() {
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setSwapInterval(0); // Disable vertical refresh syncing
    QSurfaceFormat::setDefaultFormat(format);
}

int main(int argc, char *argv[]) {
    setDefaultFormat();
    Application::setAttribute(Qt::AA_ShareOpenGLContexts);
    Application a(argc, argv);

    auto prevPath = SPtrCreate(SmartPath)();
    QPointF pt(0, 0);
    prevPath->actionAddFirstNode(pt, pt, pt);
    pt.setY(1);
    prevPath->actionAddNormalNodeAtEnd(0, pt, pt, pt);
    pt.setX(1);
    prevPath->actionAddNormalNodeAtEnd(1, pt, pt, pt);
    pt.setY(0);
    prevPath->actionAddNormalNodeAtEnd(2, pt, pt, pt);
    prevPath->actionConnectNodes(0, 3);

    pt.setX(2);
    pt.setY(1);
    prevPath->actionAddFirstNode(pt, pt, pt);
    pt.setX(3);
    prevPath->actionAddNormalNodeAtEnd(4, pt, pt, pt);
    pt.setY(0);
    prevPath->actionAddNormalNodeAtEnd(5, pt, pt, pt);
    pt.setX(2);
    prevPath->actionAddNormalNodeAtEnd(6, pt, pt, pt);
    prevPath->actionConnectNodes(4, 7);

    prevPath->getPathAt();

    MainWindow w;
#ifdef QT_DEBUG
    qint64 pId = QCoreApplication::applicationPid();
    QProcess *process = new QProcess(&w);
    process->start("prlimit --data=4500000000 --pid " + QString::number(pId));
#endif

    w.show();

    return a.exec();
}
