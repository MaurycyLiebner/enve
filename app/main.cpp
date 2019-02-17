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

    MainWindow w;
#ifdef QT_DEBUG
    qint64 pId = QCoreApplication::applicationPid();
    QProcess *process = new QProcess(&w);
    process->start("prlimit --data=4500000000 --pid " + QString::number(pId));
#endif

    auto prevPath = SmartPath();
    QPointF pt(0, 0);
    prevPath.actionAddFirstNode(pt, pt, pt);
    pt.setY(1);
    prevPath.actionInsertNodeBetween(0, 1, pt, pt, pt);
    pt.setX(1);
    prevPath.actionInsertNodeBetween(1, 2, pt, pt, pt);
    pt.setY(0);
    prevPath.actionInsertNodeBetween(2, 3, pt, pt, pt);
    prevPath.actionConnectNodes(0, 3);

    pt.setX(2);
    pt.setY(1);
    prevPath.actionAddFirstNode(pt, pt, pt);
    pt.setX(3);
    prevPath.actionInsertNodeBetween(4 + 1, 5 + 1, pt, pt, pt);
    pt.setY(0);
    prevPath.actionInsertNodeBetween(5 + 1, 6 + 1, pt, pt, pt);
    pt.setX(2);
    prevPath.actionInsertNodeBetween(6 + 1, 7 + 1, pt, pt, pt);
    prevPath.actionConnectNodes(4 + 1, 7 + 1);

    //prevPath.getPathAt();

    auto nextPath = SmartPath(prevPath);

    prevPath.setNext(&nextPath);
    nextPath.setPrev(&prevPath);

    nextPath.interpolateWithPrev(0.5);
    prevPath.interpolateWithNext(0.5);

    qDebug() << "DISCONNECT";
    nextPath.actionDisconnectNodes(2, 3);

    nextPath.interpolateWithPrev(0.5);
    prevPath.interpolateWithNext(0.5);
//    qDebug() << "prevPath at:";
//    prevPath.getPathAt();
//    qDebug() << "nextPath at:";
//    nextPath.getPathAt();

//    qDebug() << "prevPath for next:";
//    prevPath.getPathForNext();
//    qDebug() << "nextPath for prev:";
//    nextPath.getPathForPrev();

    qDebug() << "ADD NODE";
    pt = QPointF(1.5, 0.5);
    nextPath.actionInsertNodeBetween(2, 3, pt, pt, pt);
    nextPath.interpolateWithPrev(0.5);
    prevPath.interpolateWithNext(0.5);

    qDebug() << "CONNECT";
    nextPath.actionConnectNodes(3, 5);
    nextPath.interpolateWithPrev(0.5);
    prevPath.interpolateWithNext(0.5);

    qDebug() << "INSERT NODE";
    nextPath.actionInsertNodeBetween(0, 1, 0.5);
    nextPath.interpolateWithPrev(0.5);
    prevPath.interpolateWithNext(0.5);

    qDebug() << "REMOVE NODE";
    prevPath.actionRemoveNormalNode(3);
    nextPath.interpolateWithPrev(0.5);
    prevPath.interpolateWithNext(0.5);

    qDebug() << "DISCONNECT";
    prevPath.actionDisconnectNodes(3, 4);
    nextPath.interpolateWithPrev(0.5);
    prevPath.interpolateWithNext(0.5);

    w.show();

    return a.exec();
}
