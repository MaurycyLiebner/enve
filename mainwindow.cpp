#include "mainwindow.h"
#include "canvas.h"
#include <QKeyEvent>
#include <QApplication>
#include <QDebug>
#include <QToolBar>
#include "updatescheduler.h"
#include "Colors/ColorWidgets/colorsettingswidget.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    processKeyEvent(event);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QApplication::instance()->installEventFilter((QObject*)this);
    setStyleSheet(
        "QMainWindow::separator {\
            background: rgb(155, 155, 155);\
            width: 10px;\
            height: 10px;\
        }");

    mRightDock = new QDockWidget(this);
    mFillStrokeSettings = new FillStrokeSettingsWidget(this);

    mCanvas = new Canvas(mFillStrokeSettings, this);

    mRightDock->setWidget(mFillStrokeSettings);
    mRightDock->setFeatures(QDockWidget::DockWidgetVerticalTitleBar	);
    addDockWidget(Qt::RightDockWidgetArea, mRightDock);

    mToolBar = new QToolBar(this);
    mActionConnectPoints = new QAction("CONNECT POINTS", this);
    mToolBar->addAction(mActionConnectPoints);
    connect(mActionConnectPoints, SIGNAL(triggered(bool)),
            mCanvas, SLOT(connectPointsSlot()) );

    mActionDisconnectPoints = new QAction("DISCONNECT POINTS", this);
    mToolBar->addAction(mActionDisconnectPoints);
    connect(mActionDisconnectPoints, SIGNAL(triggered(bool)),
            mCanvas, SLOT(disconnectPointsSlot()) );

    mActionMergePoints = new QAction("MERGE POINTS", this);
    mToolBar->addAction(mActionMergePoints);
    connect(mActionMergePoints, SIGNAL(triggered(bool)),
            mCanvas, SLOT(mergePointsSlot()) );
//
    mActionSymmetricPointCtrls = new QAction("SYMMETRIC POINTS", this);
    mToolBar->addAction(mActionSymmetricPointCtrls);
    connect(mActionSymmetricPointCtrls, SIGNAL(triggered(bool)),
            mCanvas, SLOT(makePointCtrlsSymmetric()) );

    mActionSmoothPointCtrls = new QAction("SMOOTH POINTS", this);
    mToolBar->addAction(mActionSmoothPointCtrls);
    connect(mActionSmoothPointCtrls, SIGNAL(triggered(bool)),
            mCanvas, SLOT(makePointCtrlsSmooth()) );

    mActionCornerPointCtrls = new QAction("CORNER POINTS", this);
    mToolBar->addAction(mActionCornerPointCtrls);
    connect(mActionCornerPointCtrls, SIGNAL(triggered(bool)),
            mCanvas, SLOT(makePointCtrlsCorner()) );
//
    addToolBar(mToolBar);

    setCentralWidget(mCanvas);
}

MainWindow::~MainWindow()
{

}

UndoRedoStack *MainWindow::getUndoRedoStack()
{
    return &mUndoRedoStack;
}

void MainWindow::addUpdateScheduler(UpdateScheduler *scheduler)
{
    mUpdateSchedulers.prepend(scheduler);
}

void MainWindow::callUpdateSchedulers()
{
    foreach(UpdateScheduler *sheduler, mUpdateSchedulers) {
        sheduler->update();
        delete sheduler;
    }
    mUpdateSchedulers.clear();
    mCanvas->updatePivotIfNeeded();
    mCanvas->repaintIfNeeded();
}

FillStrokeSettingsWidget *MainWindow::getFillStrokeSettings() {
    return mFillStrokeSettings;
}

void MainWindow::scheduleRepaint()
{
    mCanvas->scheduleRepaint();
}

void MainWindow::schedulePivotUpdate()
{
    mCanvas->schedulePivotUpdate();
}

Canvas *MainWindow::getCanvas()
{
    return mCanvas;
}

bool MainWindow::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent *key_event = (QKeyEvent*)e;
        return processKeyEvent(key_event);
    }
    return false;
}

bool isCtrlPressed() {
    return (QApplication::keyboardModifiers() & Qt::ControlModifier);
}

bool MainWindow::processKeyEvent(QKeyEvent *event) {
    bool returnBool = true;
    if(event->key() == Qt::Key_Z &&
            isCtrlPressed()) {
        if(QApplication::keyboardModifiers() & Qt::ShiftModifier) {
            mUndoRedoStack.redo();
        } else {
            mUndoRedoStack.undo();
        }
    } else if(isCtrlPressed() && event->key() == Qt::Key_S) {
        saveToFile("test.av");
    } else if(isCtrlPressed() && event->key() == Qt::Key_O) {
        loadFile("test.av");
    } else {
        returnBool = mCanvas->processKeyEvent(event);
    }
    mCanvas->schedulePivotUpdate();

    callUpdateSchedulers();
    return returnBool;
}

void MainWindow::loadFile(QString path) {
    QFile file(path);
    if(!file.exists()) {
        return;
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");//not dbConnection
    db.setDatabaseName(path);
    db.open();

    mFillStrokeSettings->loadAllGradientsFromSql();
    mCanvas->loadAllBoxesFromSql();

    db.close();
}

void MainWindow::saveToFile(QString path) {
    QFile file(path);
    if(file.exists()) {
        file.remove();
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");//not dbConnection
    db.setDatabaseName(path);
    db.open();
    QSqlQuery query;
    query.exec("CREATE TABLE color "
               "(id INTEGER PRIMARY KEY, "
               "hue REAL, "
               "saturation REAL, "
               "value REAL, "
               "alpha REAL )");
    query.exec("CREATE TABLE gradient "
               "(id INTEGER PRIMARY KEY)");
    query.exec("CREATE TABLE gradientcolor "
               "(colorid INTEGER, "
               "gradientid INTEGER, "
               "positioningradient INTEGER, "
               "FOREIGN KEY(colorid) REFERENCES color(id), "
               "FOREIGN KEY(gradientid) REFERENCES gradient(id) )");
    query.exec("CREATE TABLE paintsettings "
               "(id INTEGER PRIMARY KEY, "
               "painttype INTEGER, "
               "colorid INTEGER, "
               "gradientid INTEGER, "
               "FOREIGN KEY(colorid) REFERENCES color(id), "
               "FOREIGN KEY(gradientid) REFERENCES gradient(id) )");
    query.exec("CREATE TABLE strokesettings "
               "(id INTEGER PRIMARY KEY, "
               "linewidth REAL, "
               "capstyle INTEGER, "
               "joinstyle INTEGER, "
               "paintsettingsid INTEGER, "
               "FOREIGN KEY(paintsettingsid) REFERENCES paintsettings(id) )");

    query.exec("CREATE TABLE boundingbox "
               "(id INTEGER PRIMARY KEY, "
               "boxtype INTEGER, "
               "m11_trans REAL, "
               "m12_trans REAL, "
               "m21_trans REAL, "
               "m22_trans REAL, "
               "dx_trans REAL, "
               "dy_trans REAL, "
               "pivotx REAL, "
               "pivoty REAL, "
               "parentboundingboxid INTEGER, "
               "FOREIGN KEY(parentboundingboxid) REFERENCES boundingbox(id) )");
    query.exec("CREATE TABLE boxesgroup "
               "(id INTEGER PRIMARY KEY, "
               "boundingboxid INTEGER, "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");
    query.exec("CREATE TABLE vectorpath "
               "(id INTEGER PRIMARY KEY, "
               "fillgradientstartx REAL, "
               "fillgradientstarty REAL, "
               "fillgradientendx REAL, "
               "fillgradientendy REAL, "
               "strokegradientstartx REAL, "
               "strokegradientstarty REAL, "
               "strokegradientendx REAL, "
               "strokegradientendy REAL, "
               "boundingboxid INTEGER, "
               "fillsettingsid INTEGER, "
               "strokesettingsid INTEGER, "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id), "
               "FOREIGN KEY(fillsettingsid) REFERENCES paintsettings(id), "
               "FOREIGN KEY(strokesettingsid) REFERENCES strokesettings(id) )");
    query.exec("CREATE TABLE pathpoint "
               "(id INTEGER PRIMARY KEY, "
               "isfirst BOOLEAN, "
               "isendpoint BOOLEAN, "
               "xrelpos REAL, "
               "yrelpos REAL, "
               "startctrlptrelx REAL, "
               "startctrlptrely REAL, "
               "endctrlptrelx REAL, "
               "endctrlptrely REAL, "
               "vectorpathid INTEGER, "
               "FOREIGN KEY(vectorpathid) REFERENCES vectorpath(id) )");
    mFillStrokeSettings->saveGradientsToQuery();
    mCanvas->saveToQuery();
    /*query.exec("INSERT INTO color (hue, saturation, value, alpha) VALUES (0.1, 0.2, 0.3, 0.4)");
    int id = query.lastInsertId().toInt();
    qDebug() << id;
    query.exec(QString("INSERT INTO color (hue, saturation, value, alpha) VALUES (%1, 0.4, 0.1, 0.2)").arg(0.1121212f, 0, 'f'));
    id = query.lastInsertId().toInt();
    qDebug() << id;*/
    db.close();
}
