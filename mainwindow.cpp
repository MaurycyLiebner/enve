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
#include <QMenuBar>
#include <QFileDialog>

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    processKeyEvent(event);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    mUndoRedoStack.setWindow(this);
    setCurrentPath("");
    QSqlDatabase::addDatabase("QSQLITE");
    QApplication::instance()->installEventFilter((QObject*)this);
    setStyleSheet(
        "QMainWindow::separator {\
            background: rgb(0, 0, 0);\
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
    mActionConnectPoints = new QAction(
                QIcon("pixmaps/icons/ink_node_join.png"),
                "CONNECT POINTS", this);
    mToolBar->addAction(mActionConnectPoints);
    connect(mActionConnectPoints, SIGNAL(triggered(bool)),
            mCanvas, SLOT(connectPointsSlot()) );

    mActionDisconnectPoints = new QAction(
                QIcon("pixmaps/icons/ink_node_delete_segment.png"),
                "DISCONNECT POINTS", this);
    mToolBar->addAction(mActionDisconnectPoints);
    connect(mActionDisconnectPoints, SIGNAL(triggered(bool)),
            mCanvas, SLOT(disconnectPointsSlot()) );

    mActionMergePoints = new QAction(
                QIcon("pixmaps/icons/ink_node_join.png"),
                "MERGE POINTS", this);
    mToolBar->addAction(mActionMergePoints);
    connect(mActionMergePoints, SIGNAL(triggered(bool)),
            mCanvas, SLOT(mergePointsSlot()) );
//
    mActionSymmetricPointCtrls = new QAction(
                QIcon("pixmaps/icons/ink_node_symmetric.png"),
                "SYMMETRIC POINTS", this);
    mToolBar->addAction(mActionSymmetricPointCtrls);
    connect(mActionSymmetricPointCtrls, SIGNAL(triggered(bool)),
            mCanvas, SLOT(makePointCtrlsSymmetric()) );

    mActionSmoothPointCtrls = new QAction(
                QIcon("pixmaps/icons/ink_node_smooth.png"),
                "SMOOTH POINTS", this);
    mToolBar->addAction(mActionSmoothPointCtrls);
    connect(mActionSmoothPointCtrls, SIGNAL(triggered(bool)),
            mCanvas, SLOT(makePointCtrlsSmooth()) );

    mActionCornerPointCtrls = new QAction(
                QIcon("pixmaps/icons/ink_node_cusp.png"),
                "CORNER POINTS", this);
    mToolBar->addAction(mActionCornerPointCtrls);
    connect(mActionCornerPointCtrls, SIGNAL(triggered(bool)),
            mCanvas, SLOT(makePointCtrlsCorner()) );
//
    addToolBar(mToolBar);


//
    mMenuBar = new QMenuBar(this);

    mFileMenu = mMenuBar->addMenu("File");
    mFileMenu->addAction("Open...", this, SLOT(openFile()));
    mFileMenu->addAction("Import...", this, SLOT(importFile()));
    mFileMenu->addAction("Export Selected...", this, SLOT(exportSelected()));
    mFileMenu->addAction("Revert", this, SLOT(revert()));
    mFileMenu->addSeparator();
    mFileMenu->addAction("Save", this, SLOT(saveFile()));
    mFileMenu->addAction("Save As...", this, SLOT(saveFileAs()));
    mFileMenu->addAction("Save Backup", this, SLOT(saveBackup()));
    mFileMenu->addSeparator();
    mFileMenu->addAction("Close", this, SLOT(closeProject()));
    mFileMenu->addSeparator();
    mFileMenu->addAction("Exit", this, SLOT(exitProgram()));

    setMenuBar(mMenuBar);
//

    setCentralWidget(mCanvas);

    showMaximized();
}

MainWindow::~MainWindow()
{

}

UndoRedoStack *MainWindow::getUndoRedoStack()
{
    return &mUndoRedoStack;
}

void MainWindow::setFileChangedSinceSaving(bool changed) {
    if(changed == mChangedSinceSaving) return;
    mChangedSinceSaving = changed;
    updateTitle();
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

void MainWindow::disableEventFilter() {
    mEventFilterDisabled = true;
}

void MainWindow::enableEventFilter() {
    mEventFilterDisabled = false;
}

bool MainWindow::eventFilter(QObject *, QEvent *e)
{
    if(mEventFilterDisabled) {
        return false;
    }
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
        saveFile();
    } else if(isCtrlPressed() && event->key() == Qt::Key_O) {
        openFile();
    } else {
        returnBool = mCanvas->processFilteredKeyEvent(event);
    }
    mCanvas->schedulePivotUpdate();

    callUpdateSchedulers();
    return returnBool;
}

void MainWindow::clearAll() {
    foreach(UpdateScheduler *sheduler, mUpdateSchedulers) {
        delete sheduler;
    }
    mUpdateSchedulers.clear();

    mUndoRedoStack.clearAll();
    mCanvas->clearAll();
    mFillStrokeSettings->clearAll();
}

void MainWindow::exportSelected(QString path) {
    setDisabled(true);

    QFile file(path);
    if(file.exists()) {
        file.remove();
    }

    QSqlDatabase db = QSqlDatabase::database();
    db.setDatabaseName(path);
    db.open();

    createTablesInSaveDatabase();

    mFillStrokeSettings->saveGradientsToSqlIfPathSelected();
    mCanvas->saveSelectedToSqlForCurrentBox();

    db.close();

    setEnabled(true);
}

void MainWindow::setCurrentPath(QString newPath) {
    mCurrentFilePath = newPath;
    updateTitle();
}

void MainWindow::updateTitle() {
    QString star = "";
    if(mChangedSinceSaving) star = "*";
    setWindowTitle(mCurrentFilePath.split("/").last() + star + " - AniVect");
}

void MainWindow::openFile()
{
    disableEventFilter();
    QString openPath = QFileDialog::getOpenFileName(this,
        "Open File", "", "AniVect Files (*.av)");
    enableEventFilter();
    if(!openPath.isEmpty()) {
        setCurrentPath(openPath);
        loadFile(mCurrentFilePath);
    }
    setFileChangedSinceSaving(false);
}

void MainWindow::saveFile()
{
    if(mCurrentFilePath.isEmpty()) {
        saveFileAs();
        return;
    }
    saveToFile(mCurrentFilePath);
    setFileChangedSinceSaving(false);
}

void MainWindow::saveFileAs()
{
    disableEventFilter();
    QString saveAs = QFileDialog::getSaveFileName(this, "Save File",
                               "untitled.av",
                               "AniVect Files (*.av)");
    enableEventFilter();
    if(!saveAs.isEmpty()) {
        setCurrentPath(saveAs);
        saveToFile(mCurrentFilePath);
    }
    setFileChangedSinceSaving(false);
}

void MainWindow::saveBackup()
{
    QString backupPath = "backup/backup_%1.av";
    int id = 1;
    QFile backupFile(backupPath.arg(id));
    while(backupFile.exists()) {
        id++;
        backupFile.setFileName(backupPath.arg(id) );
    }
    saveToFile(backupPath.arg(id));
}

void MainWindow::closeProject()
{
    setCurrentPath("");
    clearAll();
    setFileChangedSinceSaving(false);
}

void MainWindow::exitProgram()
{
    close();
}

void MainWindow::importFile()
{
    disableEventFilter();
    QString importPath = QFileDialog::getOpenFileName(this,
        "Open File", "", "AniVect Files (*.av)");
    enableEventFilter();
    if(!importPath.isEmpty()) {
        importFile(importPath);
    }
}

void MainWindow::exportSelected()
{
    disableEventFilter();
    QString saveAs = QFileDialog::getSaveFileName(this, "Export Selected",
                               "untitled.av",
                               "AniVect Files (*.av)");
    enableEventFilter();
    if(!saveAs.isEmpty()) {
        exportSelected(saveAs);
    }
}

void MainWindow::revert()
{
    loadFile(mCurrentFilePath);
    setFileChangedSinceSaving(false);
}

void MainWindow::importFile(QString path) {
    setDisabled(true);
    mUndoRedoStack.startNewSet();

    QFile file(path);
    if(!file.exists()) {
        return;
    }
    QSqlDatabase db = QSqlDatabase::database();//not dbConnection
    db.setDatabaseName(path);
    db.open();

    mFillStrokeSettings->loadAllGradientsFromSql();
    mCanvas->loadAllBoxesFromSql();

    db.close();
    mUndoRedoStack.finishSet();
    setEnabled(true);
    scheduleRepaint();
    callUpdateSchedulers();
}

void MainWindow::loadFile(QString path) {
    clearAll();

    importFile(path);

    mUndoRedoStack.clearAll();
    setFileChangedSinceSaving(false);
}

void MainWindow::createTablesInSaveDatabase() {
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
               "pivotchanged BOOLEAN, "
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
}

void MainWindow::saveToFile(QString path) {
    setDisabled(true);
    QFile file(path);
    if(file.exists()) {
        file.remove();
    }

    QSqlDatabase db = QSqlDatabase::database();
    db.setDatabaseName(path);
    db.open();

    createTablesInSaveDatabase();

    mFillStrokeSettings->saveGradientsToQuery();
    mCanvas->saveToSql();
    db.close();

    setEnabled(true);
}
