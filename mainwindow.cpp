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
#include <QMessageBox>
#include "boxeslist.h"
#include "boxeslistanimationdockwidget.h"
#include "paintcontroler.h"
#include "qdoubleslider.h"
#include "renderoutputwidget.h"
#include "svgimporter.h"

MainWindow *MainWindow::mMainWindowInstance;

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    processKeyEvent(event);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    BrushStroke::loadStrokePixmaps();
    mMainWindowInstance = this;
    //int nThreads = QThread::idealThreadCount();
    mPaintControlerThread = new QThread(this);
    mPaintControler = new PaintControler();
    mPaintControler->moveToThread(mPaintControlerThread);
    connect(mPaintControler, SIGNAL(finishedUpdatingLastBox()),
            this, SLOT(sendNextBoxForUpdate()) );
    connect(this, SIGNAL(updateBox(BoundingBox*)),
            mPaintControler, SLOT(updateBoxPixmaps(BoundingBox*)) );
    mPaintControlerThread->start();

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
    mRightDock->setWidget(mFillStrokeSettings);
    mRightDock->setFeatures(0);
    mRightDock->setTitleBarWidget(new QWidget());
    addDockWidget(Qt::RightDockWidgetArea, mRightDock);

    mCanvas = new Canvas(mFillStrokeSettings, this);

    mFillStrokeSettings->setCanvasPtr(mCanvas);

    mBottomDock = new QDockWidget(this);
    mBoxesListAnimationDockWidget = new BoxesListAnimationDockWidget(this);
    mBoxListWidget = mBoxesListAnimationDockWidget->getBoxesList();
    mKeysView = mBoxesListAnimationDockWidget->getKeysView();
    mBottomDock->setWidget(mBoxesListAnimationDockWidget);

    mBottomDock->setFeatures(0);
    mBottomDock->setTitleBarWidget(new QWidget());
    addDockWidget(Qt::BottomDockWidgetArea, mBottomDock);


    mToolBar = new QToolBar(this);

    mMovePathMode = new QAction(
                QIcon("pixmaps/icons/ink_draw_select.png"),
                "CONNECT POINTS", this);
    mMovePathMode->setToolTip("F1");
    mMovePathMode->setCheckable(true);
    mMovePathMode->setChecked(true);
    mToolBar->addAction(mMovePathMode);
    connect(mMovePathMode, SIGNAL(triggered(bool)),
            mCanvas, SLOT(setMovePathMode()) );

    mMovePointMode = new QAction(
                QIcon("pixmaps/icons/ink_draw_node.png"),
                "CONNECT POINTS", this);
    mMovePointMode->setToolTip("F2");
    mMovePointMode->setCheckable(true);
    mToolBar->addAction(mMovePointMode);
    connect(mMovePointMode, SIGNAL(triggered(bool)),
            mCanvas, SLOT(setMovePointMode()) );

    mAddPointMode = new QAction(
                QIcon("pixmaps/icons/ink_draw_pen.png"),
                "CONNECT POINTS", this);
    mAddPointMode->setToolTip("F3");
    mAddPointMode->setCheckable(true);
    mToolBar->addAction(mAddPointMode);
    connect(mAddPointMode, SIGNAL(triggered(bool)),
            mCanvas, SLOT(setAddPointMode()) );

    mCircleMode = new QAction(
                QIcon("pixmaps/icons/ink_draw_arc.png"),
                "CONNECT POINTS", this);
    mCircleMode->setToolTip("F4");
    mCircleMode->setCheckable(true);
    mToolBar->addAction(mCircleMode);
    connect(mCircleMode, SIGNAL(triggered(bool)),
            mCanvas, SLOT(setCircleMode()) );

    mRectangleMode = new QAction(
                QIcon("pixmaps/icons/ink_draw_rect.png"),
                "CONNECT POINTS", this);
    mRectangleMode->setToolTip("F5");
    mRectangleMode->setCheckable(true);
    mToolBar->addAction(mRectangleMode);
    connect(mRectangleMode, SIGNAL(triggered(bool)),
            mCanvas, SLOT(setRectangleMode()) );

    mTextMode = new QAction(
                QIcon("pixmaps/icons/ink_draw_text.png"),
                "CONNECT POINTS", this);
    mTextMode->setToolTip("F6");
    mTextMode->setCheckable(true);
    mToolBar->addAction(mTextMode);
    connect(mTextMode, SIGNAL(triggered(bool)),
            mCanvas, SLOT(setTextMode()) );


    mToolBar->addSeparator();

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

    mFontWidget = new FontsWidget(this);
    mToolBar->addWidget(mFontWidget);

    connect(mFontWidget, SIGNAL(fontSizeChanged(qreal)),
            mCanvas, SLOT(setFontSize(qreal)) );
    connect(mFontWidget, SIGNAL(fontFamilyAndStyleChanged(QString, QString)),
            mCanvas, SLOT(setFontFamilyAndStyle(QString, QString)) );

//
    mMenuBar = new QMenuBar(this);

    mFileMenu = mMenuBar->addMenu("File");
    mFileMenu->addAction("New...", this, SLOT(newFile()));
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
    mFileMenu->addAction("Exit", this, SLOT(close()));

    mEditMenu = mMenuBar->addMenu("Edit");

    mEditMenu->addAction("Undo");
    mEditMenu->addAction("Redo");
    mEditMenu->addAction("Undo History...");
    mEditMenu->addSeparator();
    mEditMenu->addAction("Cut");
    mEditMenu->addAction("Copy");
    mEditMenu->addAction("Paste");
    mEditMenu->addSeparator();
    mEditMenu->addAction("Duplicate");
    mEditMenu->addAction("Clone");
    mEditMenu->addSeparator();
    mEditMenu->addAction("Delete");
    mEditMenu->addSeparator();
    mEditMenu->addAction("Select All");
    mEditMenu->addAction("Invert Selection");
    mEditMenu->addAction("Deselect");

    mSelectSameMenu = mEditMenu->addMenu("Select Same");
    mSelectSameMenu->addAction("Fill and Stroke");
    mSelectSameMenu->addAction("Fill Color");
    mSelectSameMenu->addAction("Stroke Color");
    mSelectSameMenu->addAction("Stroke Style");
    mSelectSameMenu->addAction("Object Type");

    mObjectMenu = mMenuBar->addMenu("Object");

    mObjectMenu->addSeparator();
    mObjectMenu->addAction("Raise", mCanvas, SLOT(raiseAction()));
    mObjectMenu->addAction("Lower", mCanvas, SLOT(lowerAction()));
    mObjectMenu->addAction("Rasie to Top", mCanvas, SLOT(raiseToTopAction()));
    mObjectMenu->addAction("Lower to Bottom", mCanvas, SLOT(lowerToBottomAction()));
    mObjectMenu->addSeparator();
    mObjectMenu->addAction("Rotate 90° Right");
    mObjectMenu->addAction("Rotate 90° Left");
    mObjectMenu->addAction("Flip Horizontal");
    mObjectMenu->addAction("Flip Vertical");
    mObjectMenu->addSeparator();
    mObjectMenu->addAction("Group");
    mObjectMenu->addAction("Ungroup");

    mPathMenu = mMenuBar->addMenu("Path");

    mPathMenu->addAction("Object to Path", mCanvas, SLOT(objectsToPathAction()));
    mPathMenu->addAction("Stroke to Path");
    mPathMenu->addSeparator();
    mPathMenu->addAction("Union");
    mPathMenu->addAction("Difference");
    mPathMenu->addAction("Intersection");
    mPathMenu->addAction("Exclusion");
    mPathMenu->addAction("Division");
    mPathMenu->addAction("Cut Path");
    mPathMenu->addSeparator();
    mPathMenu->addAction("Combine");
    mPathMenu->addAction("Break Apart");

    mEffectsMenu = mMenuBar->addMenu("Effects");

    mEffectsMenu->addAction("Blur");

    mViewMenu = mMenuBar->addMenu("View");
    mActionHighQualityView = mViewMenu->addAction("High Quality");
    mActionHighQualityView->setCheckable(true);
    mActionHighQualityView->setChecked(false);
    connect(mActionHighQualityView, SIGNAL(toggled(bool)),
            this, SLOT(setHighQualityView(bool)));

    mActionEffectsPaintEnabled = mViewMenu->addAction("Effects");
    mActionEffectsPaintEnabled->setCheckable(true);
    mActionEffectsPaintEnabled->setChecked(true);
    mActionEffectsPaintEnabled->setShortcut(QKeySequence(Qt::Key_E));
    connect(mActionEffectsPaintEnabled, SIGNAL(toggled(bool)),
            this, SLOT(setEffectsPaintEnabled(bool)));

    mRenderMenu = mMenuBar->addMenu("Render");
    mRenderMenu->addAction("Render", this, SLOT(renderOutput()));

    setMenuBar(mMenuBar);
//

    setCentralWidget(mCanvas);

    showMaximized();

    QDockWidget *shapesMenuWidget = new QDockWidget(this);
    mVectorShapesMenu = new VectorShapesMenu(this);
    shapesMenuWidget->setWidget(mVectorShapesMenu);
    addDockWidget(Qt::RightDockWidgetArea, shapesMenuWidget);

    loadSVGFile("/home/ailuropoda/example_svg.svg", mCanvas);
}

MainWindow::~MainWindow()
{
    //mPaintControlerThread->terminate();
    mPaintControlerThread->quit();
}

MainWindow *MainWindow::getInstance()
{
    return mMainWindowInstance;
}

void MainWindow::updateCanvasModeButtonsChecked(CanvasMode currentMode) {
    mMovePathMode->setChecked(currentMode == MOVE_PATH);
    mMovePointMode->setChecked(currentMode == MOVE_POINT);
    mAddPointMode->setChecked(currentMode == ADD_POINT);
    mCircleMode->setChecked(currentMode == ADD_CIRCLE);
    mRectangleMode->setChecked(currentMode == ADD_RECTANGLE);
    mTextMode->setChecked(currentMode == ADD_TEXT);
}

void MainWindow::addBoxAwaitingUpdate(BoundingBox *box)
{
    if(mNoBoxesAwaitUpdate) {
        mNoBoxesAwaitUpdate = false;
        mLastUpdatedBox = box;
        emit updateBox(box);
    } else {
        mBoxesAwaitingUpdate << box;
    }
}

void MainWindow::sendNextBoxForUpdate()
{
    if(mLastUpdatedBox != NULL) {
        mLastUpdatedBox->setAwaitingUpdate(false);
        if(mLastUpdatedBox->shouldRedoUpdate()) {
            mLastUpdatedBox->setRedoUpdateToFalse();
            mLastUpdatedBox->awaitUpdate();
        }
    }
    if(mBoxesAwaitingUpdate.isEmpty()) {
        mNoBoxesAwaitUpdate = true;
        mLastUpdatedBox = NULL;
        callUpdateSchedulers();
    } else {
        mLastUpdatedBox = mBoxesAwaitingUpdate.takeFirst();
        emit updateBox(mLastUpdatedBox);
    }
}

void MainWindow::playPreview()
{
    mCanvas->clearPreview();
    mSavedCurrentFrame = mCurrentFrame;
    mCanvas->updateRenderRect();
    for(int i = mSavedCurrentFrame; i <= mMaxFrame; i++) {
        mBoxesListAnimationDockWidget->setCurrentFrame(i);
        mCanvas->renderCurrentFrameToPreview();
    }
    mBoxesListAnimationDockWidget->setCurrentFrame(mSavedCurrentFrame);

    mCanvas->playPreview();
}

void MainWindow::saveOutput(QString renderDest)
{
    mSavedCurrentFrame = mCurrentFrame;
    for(int i = mMinFrame; i <= mMaxFrame; i++) {
        mBoxesListAnimationDockWidget->setCurrentFrame(i);
        mCanvas->renderCurrentFrameToOutput(renderDest);
    }
    mBoxesListAnimationDockWidget->setCurrentFrame(mSavedCurrentFrame);
}

void MainWindow::previewFinished() {
    mBoxesListAnimationDockWidget->previewFinished();
}

void MainWindow::stopPreview() {
    mCanvas->clearPreview();
    mCanvas->repaint();
}

void MainWindow::renderOutput()
{
    RenderOutputWidget *dialog = new RenderOutputWidget(this);
    connect(dialog, SIGNAL(render(QString)), this, SLOT(saveOutput(QString)));
    dialog->exec();
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

bool MainWindow::isShiftPressed()
{
    return mShiftPressed;
}

bool MainWindow::isCtrlPressed()
{
    return mCtrlPressed;
}

bool MainWindow::isAltPressed()
{
    return mAltPressed;
}

void MainWindow::callUpdateSchedulers()
{
    mKeysView->graphUpdateAfterKeysChangedIfNeeded();

    foreach(UpdateScheduler *sheduler, mUpdateSchedulers) {
        sheduler->update();
        delete sheduler;
    }

    mUpdateSchedulers.clear();
    mCanvas->updatePivotIfNeeded();
    mCanvas->repaint();
    mBoxListWidget->repaint();
    mKeysView->repaint();
    updateDisplayedFillStrokeSettingsIfNeeded();
    mFillStrokeSettings->repaint();
}

void MainWindow::setCurrentShapesMenuBox(BoundingBox *box) {
    if(mVectorShapesMenu == NULL) return;
    mVectorShapesMenu->setSelectedBoundingBox(box);
}

FillStrokeSettingsWidget *MainWindow::getFillStrokeSettings() {
    return mFillStrokeSettings;
}

bool MainWindow::askForSaving() {
    if(mChangedSinceSaving) {
        int buttonId = QMessageBox::question(this, "Save", "Save changes to document \"" +
                                      mCurrentFilePath.split("/").last() +
                                      "\"?", "Close without saving",
                                      "Cancel",
                                      "Save");
        if (buttonId == 1) {
            return false;
        } else {
            if(buttonId == 2) {
                saveFile();
            }
            return true;
        }
    }
    return true;
}

void MainWindow::schedulePivotUpdate()
{
    mCanvas->schedulePivotUpdate();
}

KeysView *MainWindow::getKeysView()
{
    return mKeysView;
}

BoxesList *MainWindow::getBoxesList()
{
    return mBoxesListAnimationDockWidget->getBoxesList();
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

void MainWindow::disable()
{
    grayOutWidget = new QWidget(this);
    grayOutWidget->setFixedSize(size());
    grayOutWidget->setStyleSheet("QWidget { background-color: rgb(0, 0, 0, 125) }");
    grayOutWidget->show();
    grayOutWidget->repaint();
}

void MainWindow::enable()
{
    if(grayOutWidget == NULL) return;
    delete grayOutWidget;
    grayOutWidget = NULL;
}

int MainWindow::getCurrentFrame()
{
    return mCurrentFrame;
}

bool MainWindow::isRecording()
{
    return mRecording;
}

bool MainWindow::isRecordingAllPoints()
{
    return mAllPointsRecording;
}

int MainWindow::getMinFrame()
{
    return mMinFrame;
}

int MainWindow::getMaxFrame()
{
    return mMaxFrame;
}

void MainWindow::setCurrentFrame(int frame)
{
    mCurrentFrame = frame;
    mCanvas->updateAfterFrameChanged(mCurrentFrame);

    callUpdateSchedulers();
}

void MainWindow::setGraphEnabled(bool graphEnabled)
{
    mKeysView->setGraphViewed(graphEnabled);
}

void MainWindow::setAllPointsRecord(bool allPointsRecord)
{
    mAllPointsRecording = allPointsRecord;
}

void MainWindow::newFile()
{
    if(askForSaving()) {
        closeProject();
    }
}

bool MainWindow::eventFilter(QObject *, QEvent *e)
{
    if(mEventFilterDisabled) {
        return false;
    }
    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent *key_event = (QKeyEvent*)e;
        if(key_event->key() == Qt::Key_Control) {
            mCtrlPressed = true;
        } else if(key_event->key() == Qt::Key_Shift) {
            mShiftPressed = true;
        } else if(key_event->key() == Qt::Key_Alt) {
            mAltPressed = true;
        }
        return processKeyEvent(key_event);
    } else if(e->type() == QEvent::KeyRelease) {
        QKeyEvent *key_event = (QKeyEvent*)e;
        if(key_event->key() == Qt::Key_Control) {
            mCtrlPressed = false;
            callUpdateSchedulers();
        } else if(key_event->key() == Qt::Key_Shift) {
            mShiftPressed = false;
        } else if(key_event->key() == Qt::Key_Alt) {
            mAltPressed = false;
        }
    }
    return false;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if(!askForSaving() ) {
        e->ignore();
    }
}

void MainWindow::updateDisplayedFillStrokeSettings() {
    mCanvas->updateDisplayedFillStrokeSettings();
}

void MainWindow::updateDisplayedFillStrokeSettingsIfNeeded() {
    if(mDisplayedFillStrokeSettingsUpdateNeeded) {
        mDisplayedFillStrokeSettingsUpdateNeeded = false;
        updateDisplayedFillStrokeSettings();
    }
}

void MainWindow::scheduleDisplayedFillStrokeSettingsUpdate()
{
    mDisplayedFillStrokeSettingsUpdateNeeded = true;
}

bool MainWindow::processKeyEvent(QKeyEvent *event) {
    bool returnBool = true;
    if(event->key() == Qt::Key_Z &&
            isCtrlPressed()) {
        if(isShiftPressed()) {
            mUndoRedoStack.redo();
        } else {
            mUndoRedoStack.undo();
        }
    } else if(isCtrlPressed() && event->key() == Qt::Key_S) {
        saveFile();
    } else if(isCtrlPressed() && event->key() == Qt::Key_O) {
        openFile();
    } else if(mCanvas->processFilteredKeyEvent(event) ) {
    } else if(mBoxesListAnimationDockWidget->processFilteredKeyEvent(event) ) {
    } else {
        returnBool = false;
    }

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
    disable();

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

    enable();
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
    if(askForSaving()) {
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
    if(askForSaving()) {
        setCurrentPath("");
        clearAll();
        setFileChangedSinceSaving(false);
    }
}

void MainWindow::importFile()
{
    disableEventFilter();
    QString importPath = QFileDialog::getOpenFileName(this,
        "Open File", "", "AniVect Files (*.av)");
    enableEventFilter();
    if(!importPath.isEmpty()) {
        importFile(importPath, true);
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

void MainWindow::setHighQualityView(bool bT)
{
    if(bT) {
        mCanvas->enableHighQualityPaint();
    } else {
        mCanvas->disableHighQualityPaint();
    }

    mCanvas->updateAllBoxes();
}

void MainWindow::setEffectsPaintEnabled(bool bT)
{
    if(bT) {
        mCanvas->enableEffectsPaint();
    } else {
        mCanvas->disableEffectsPaint();
    }

    mCanvas->updateAllBoxes();
}

void MainWindow::importFile(QString path, bool loadInBox) {
    disable();
    mUndoRedoStack.startNewSet();

    QFile file(path);
    if(!file.exists()) {
        return;
    }
    QSqlDatabase db = QSqlDatabase::database();//not dbConnection
    db.setDatabaseName(path);
    db.open();

    mFillStrokeSettings->loadAllGradientsFromSql();
    mCanvas->loadAllBoxesFromSql(loadInBox);

    db.close();
    mUndoRedoStack.finishSet();
    enable();

    callUpdateSchedulers();
}

void MainWindow::loadFile(QString path) {
    clearAll();

    importFile(path, false);

    mUndoRedoStack.clearAll();
    setFileChangedSinceSaving(false);
}

void MainWindow::createTablesInSaveDatabase() {
    QSqlQuery query;

    query.exec("CREATE TABLE qrealanimator "
               "(id INTEGER PRIMARY KEY,"
               "currentvalue REAL )");

    query.exec("CREATE TABLE qrealkey "
               "(id INTEGER PRIMARY KEY, "
               "value REAL, "
               "frame INTEGER, "
               "endenabled BOOLEAN, "
               "startenabled BOOLEAN,"
               "ctrlsmode INTEGER, "
               "endvalue REAL, "
               "endframe INTEGER, "
               "startvalue REAL, "
               "startframe INTEGER, "
               "qrealanimatorid INTEGER, "
               "FOREIGN KEY(qrealanimatorid) REFERENCES qrealanimator(id) )");

    query.exec("CREATE TABLE transformanimator "
               "(id INTEGER PRIMARY KEY, "
               "posanimatorid INTEGER, "
               "scaleanimatorid INTEGER, "
               "pivotanimatorid INTEGER, "
               "rotanimatorid INTEGER, "
               "opacityanimatorid INTEGER, "
               "FOREIGN KEY(posanimatorid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(scaleanimatorid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(pivotanimatorid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(rotanimatorid) REFERENCES qrealanimator(id), "
               "FOREIGN KEY(opacityanimatorid) REFERENCES qrealanimator(id) )");

    query.exec("CREATE TABLE qpointfanimator "
               "(id INTEGER PRIMARY KEY, "
               "xanimatorid INTEGER, "
               "yanimatorid INTEGER, "
               "FOREIGN KEY(xanimatorid) REFERENCES qrealanimator(id), "
               "FOREIGN KEY(yanimatorid) REFERENCES qrealanimator(id) )");


    query.exec("CREATE TABLE coloranimator "
               "(id INTEGER PRIMARY KEY, "
               "colormode INTEGER, "
               "val1animatorid INTEGER, "
               "val2animatorid INTEGER, "
               "val3animatorid INTEGER, "
               "alphaanimatorid INTEGER, "
               "FOREIGN KEY(val1animatorid) REFERENCES qrealanimator(id), "
               "FOREIGN KEY(val2animatorid) REFERENCES qrealanimator(id), "
               "FOREIGN KEY(val3animatorid) REFERENCES qrealanimator(id), "
               "FOREIGN KEY(alphaanimatorid) REFERENCES qrealanimator(id) )");

    query.exec("CREATE TABLE gradient "
               "(id INTEGER PRIMARY KEY)");
    query.exec("CREATE TABLE gradientcolor "
               "(colorid INTEGER, "
               "gradientid INTEGER, "
               "positioningradient INTEGER, "
               "FOREIGN KEY(colorid) REFERENCES coloranimator(id), "
               "FOREIGN KEY(gradientid) REFERENCES gradient(id) )");
    query.exec("CREATE TABLE paintsettings "
               "(id INTEGER PRIMARY KEY, "
               "painttype INTEGER, "
               "colorid INTEGER, "
               "gradientid INTEGER, "
               "FOREIGN KEY(colorid) REFERENCES coloranimator(id), "
               "FOREIGN KEY(gradientid) REFERENCES gradient(id) )");
    query.exec("CREATE TABLE strokesettings "
               "(id INTEGER PRIMARY KEY, "
               "linewidthanimatorid INTEGER, "
               "capstyle INTEGER, "
               "joinstyle INTEGER, "
               "paintsettingsid INTEGER, "
               "FOREIGN KEY(linewidthanimatorid) REFERENCES qrealanimator(id), "
               "FOREIGN KEY(paintsettingsid) REFERENCES paintsettings(id) )");

    query.exec("CREATE TABLE boundingbox "
               "(id INTEGER PRIMARY KEY, "
               "name TEXT, "
               "boxtype INTEGER, "
               "transformanimatorid INTEGER, "
               "pivotchanged BOOLEAN, "
               "visible BOOLEAN, "
               "locked BOOLEAN, "
               "parentboundingboxid INTEGER, "
               "FOREIGN KEY(transformanimatorid) REFERENCES transformanimator(id), "
               "FOREIGN KEY(parentboundingboxid) REFERENCES boundingbox(id) )");
    query.exec("CREATE TABLE boxesgroup "
               "(id INTEGER PRIMARY KEY, "
               "boundingboxid INTEGER, "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");
    query.exec("CREATE TABLE pathbox "
               "(id INTEGER PRIMARY KEY, "
               "fillgradientstartid INTEGER, "
               "fillgradientendid INTEGER, "
               "strokegradientstartid INTEGER, "
               "strokegradientendid INTEGER, "
               "boundingboxid INTEGER, "
               "fillsettingsid INTEGER, "
               "strokesettingsid INTEGER, "
               "pointsinfluenceenabled BOOLEAN, "
               "FOREIGN KEY(fillgradientstartid) REFERENCES movablepoint(id), "
               "FOREIGN KEY(fillgradientendid) REFERENCES movablepoint(id), "
               "FOREIGN KEY(strokegradientstartid) REFERENCES movablepoint(id), "
               "FOREIGN KEY(strokegradientendid) REFERENCES movablepoint(id), "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id), "
               "FOREIGN KEY(fillsettingsid) REFERENCES paintsettings(id), "
               "FOREIGN KEY(strokesettingsid) REFERENCES strokesettings(id) )");
    query.exec("CREATE TABLE movablepoint "
               "(id INTEGER PRIMARY KEY, "
               "posanimatorid INTEGER, "
               "FOREIGN KEY(posanimatorid) REFERENCES qrealanimator(id) )");
    query.exec("CREATE TABLE pathpoint "
               "(id INTEGER PRIMARY KEY, "
               "isfirst BOOLEAN, "
               "isendpoint BOOLEAN, "
               "movablepointid INTEGER, "
               "startctrlptid INTEGER, "
               "endctrlptid INTEGER, "
               "boundingboxid INTEGER, "
               "ctrlsmode INTEGER, "
               "startpointenabled BOOLEAN, "
               "endpointenabled BOOLEAN, "
               "influenceanimatorid INTEGER, "
               "influencetanimatorid INTEGER, "
               "FOREIGN KEY(movablepointid) REFERENCES movablepoint(id), "
               "FOREIGN KEY(startctrlptid) REFERENCES movablepoint(id), "
               "FOREIGN KEY(endctrlptid) REFERENCES movablepoint(id), "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id), "
               "FOREIGN KEY(influenceanimatorid) REFERENCES qrealanimator(id), "
               "FOREIGN KEY(influencetanimatorid) REFERENCES qrealanimator(id) )");
    query.exec("CREATE TABLE circle "
               "(id INTEGER PRIMARY KEY, "
               "boundingboxid INTEGER, "
               "horizontalradiuspointid INTEGER, "
               "verticalradiuspointid INTEGER, "
               "FOREIGN KEY(horizontalradiuspointid) REFERENCES movablepoint(id), "
               "FOREIGN KEY(verticalradiuspointid) REFERENCES movablepoint(id), "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");

    query.exec("CREATE TABLE rectangle "
               "(id INTEGER PRIMARY KEY, "
               "boundingboxid INTEGER, "
               "topleftpointid INTEGER, "
               "bottomrightpointid INTEGER, "
               "radiuspointid INTEGER, "
               "FOREIGN KEY(topleftpointid) REFERENCES movablepoint(id), "
               "FOREIGN KEY(bottomrightpointid) REFERENCES movablepoint(id), "
               "FOREIGN KEY(radiuspointid) REFERENCES movablepoint(id), "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");

    query.exec("CREATE TABLE textbox "
               "(id INTEGER PRIMARY KEY, "
               "boundingboxid INTEGER, "
               "text TEXT, "
               "fontfamily TEXT, "
               "fontstyle TEXT, "
               "fontsize REAL, "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");

}

void MainWindow::saveToFile(QString path) {
    disable();
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

    enable();
}
