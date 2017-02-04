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
#include "boxeslistanimationdockwidget.h"
#include "paintcontroler.h"
#include "qdoubleslider.h"
#include "renderoutputwidget.h"
#include "svgimporter.h"
#include "canvaswidget.h"
#include "ObjectSettings/objectsettingswidget.h"

#include <QAudioOutput>
#include "Sound/soundcomposition.h"
#include "Sound/singlesound.h"

MainWindow *MainWindow::mMainWindowInstance;

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    processKeyEvent(event);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //mSoundComposition = new SoundComposition();
//    mSoundComposition->addSound(
//                new SingleSound("/home/ailuropoda/.Qt_pro/build-AniVect-Desktop_Qt_5_7_0_GCC_64bit-Debug/lektor.wav"));

    setupToolBar();

    mCurrentUndoRedoStack = &mUndoRedoStack;

    mMainWindowInstance = this;
    //int nThreads = QThread::idealThreadCount();
    mPaintControlerThread = new QThread(this);
    mPaintControler = new PaintControler();
    mPaintControler->moveToThread(mPaintControlerThread);
    connect(mPaintControler, SIGNAL(finishedUpdatingLastBox()),
            this, SLOT(sendNextBoxForUpdate()) );
    connect(this, SIGNAL(updateBoxPixmaps(BoundingBox*)),
            mPaintControler, SLOT(updateBoxPixmaps(BoundingBox*)) );
    mPaintControlerThread->start();

    mUndoRedoStack.setWindow(this);
    setCurrentPath("");
    QSqlDatabase::addDatabase("QSQLITE");
    QApplication::instance()->installEventFilter((QObject*)this);

    mRightDock = new QDockWidget(this);
    mFillStrokeSettings = new FillStrokeSettingsWidget(this);
    mRightDock->setWidget(mFillStrokeSettings);
    mRightDock->setFeatures(0);
    mRightDock->setTitleBarWidget(new QWidget());
    addDockWidget(Qt::RightDockWidgetArea, mRightDock);

    mBottomDock = new QDockWidget(this);

    mBottomDock->setFeatures(0);
    mBottomDock->setTitleBarWidget(new QWidget());
    addDockWidget(Qt::BottomDockWidgetArea, mBottomDock);

    mCanvasWidget = new CanvasWidget(this);

    mBoxesListAnimationDockWidget = new BoxesListAnimationDockWidget(this);
    mBoxListWidget = mBoxesListAnimationDockWidget->getBoxesList();
    mKeysView = mBoxesListAnimationDockWidget->getKeysView();
    mBottomDock->setWidget(mBoxesListAnimationDockWidget);

    mFillStrokeSettings->setCanvasWidgetPtr(mCanvasWidget);

    setupMenuBar();

    setCentralWidget(mCanvasWidget);

    showMaximized();

    QDockWidget *shapesMenuWidget = new QDockWidget(this);
    mVectorShapesMenu = new VectorShapesMenu(this);
    shapesMenuWidget->setWidget(mVectorShapesMenu);
    addDockWidget(Qt::RightDockWidgetArea, shapesMenuWidget);

    QDockWidget *effectsMenuWidget = new QDockWidget(this);
    effectsMenuWidget->setMinimumWidth(200);
    mObjectSettingsWidget = new ObjectSettingsWidget(this);
    effectsMenuWidget->setWidget(mObjectSettingsWidget);
    addDockWidget(Qt::LeftDockWidgetArea, effectsMenuWidget);

    Canvas *canvas = new Canvas(mFillStrokeSettings, mCanvasWidget);
    canvas->setName("Canvas 0");
    mCanvasWidget->addCanvasToListAndSetAsCurrent(canvas);
    mCanvas = mCanvasWidget->getCurrentCanvas();
    mCurrentCanvasComboBox->addItem(mCanvas->getName());

    connectToolBarActions();
}

MainWindow::~MainWindow()
{
    //mPaintControlerThread->terminate();
    mPaintControlerThread->quit();
}

void MainWindow::setupMenuBar() {
    mMenuBar = new QMenuBar(this);

    mFileMenu = mMenuBar->addMenu("File");
    mFileMenu->addAction("New...", this, SLOT(newFile()));
    mFileMenu->addAction("Open...", this, SLOT(openFile()));
    mFileMenu->addAction("Import...", this, SLOT(importFile()));
    mFileMenu->addAction("Link...", this, SLOT(linkFile()));
    mFileMenu->addAction("Import Animation...", this, SLOT(importAnimation()));
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
    mObjectMenu->addAction("Raise", mCanvasWidget,
                           SLOT(raiseAction()));
    mObjectMenu->addAction("Lower", mCanvasWidget,
                           SLOT(lowerAction()));
    mObjectMenu->addAction("Rasie to Top", mCanvasWidget,
                           SLOT(raiseToTopAction()));
    mObjectMenu->addAction("Lower to Bottom", mCanvasWidget,
                           SLOT(lowerToBottomAction()));
    mObjectMenu->addSeparator();
    mObjectMenu->addAction("Rotate 90° Right");
    mObjectMenu->addAction("Rotate 90° Left");
    mObjectMenu->addAction("Flip Horizontal");
    mObjectMenu->addAction("Flip Vertical");
    mObjectMenu->addSeparator();
    mObjectMenu->addAction("Group");
    mObjectMenu->addAction("Ungroup");

    mPathMenu = mMenuBar->addMenu("Path");

    mPathMenu->addAction("Object to Path", mCanvasWidget,
                         SLOT(objectsToPathAction()));
    mPathMenu->addAction("Stroke to Path");
    mPathMenu->addSeparator();
    mPathMenu->addAction("Union", mCanvasWidget,
                         SLOT(pathsUnionAction()));
    mPathMenu->addAction("Difference", mCanvasWidget,
                         SLOT(pathsDifferenceAction()));
    mPathMenu->addAction("Intersection", mCanvasWidget,
                         SLOT(pathsIntersectionAction()));
    mPathMenu->addAction("Exclusion", mCanvasWidget,
                         SLOT(pathsExclusionAction()));
    mPathMenu->addAction("Division", mCanvasWidget,
                         SLOT(pathsDivisionAction()));
//    mPathMenu->addAction("Cut Path", mCanvas,
//                         SLOT(pathsCutAction()));
    mPathMenu->addSeparator();
    mPathMenu->addAction("Combine", mCanvasWidget,
                         SLOT(pathsCombineAction()));
    mPathMenu->addAction("Break Apart", mCanvasWidget,
                         SLOT(pathsBreakApartAction()));

    mEffectsMenu = mMenuBar->addMenu("Effects");

    mEffectsMenu->addAction("Blur");

    mViewMenu = mMenuBar->addMenu("View");
    mActionHighQualityView = mViewMenu->addAction("High Quality");
    mActionHighQualityView->setCheckable(true);
    mActionHighQualityView->setChecked(false);

    mActionEffectsPaintEnabled = mViewMenu->addAction("Effects");
    mActionEffectsPaintEnabled->setCheckable(true);
    mActionEffectsPaintEnabled->setChecked(true);
    mActionEffectsPaintEnabled->setShortcut(QKeySequence(Qt::Key_E));

    mRenderMenu = mMenuBar->addMenu("Render");
    mRenderMenu->addAction("Render", this, SLOT(renderOutput()));

    setMenuBar(mMenuBar);
//

    connect(mActionHighQualityView, SIGNAL(toggled(bool)),
            mCanvasWidget, SLOT(setHighQualityView(bool)));
    connect(mActionEffectsPaintEnabled, SIGNAL(toggled(bool)),
            mCanvasWidget, SLOT(setEffectsPaintEnabled(bool)));
}

void MainWindow::updateSettingsForCurrentCanvas() {
    if(mCanvasWidget->hasNoCanvas()) return;
    Canvas *canvas = mCanvasWidget->getCurrentCanvas();
    mActionHighQualityView->setChecked(canvas->highQualityPaint());
    mActionEffectsPaintEnabled->setChecked(canvas->effectsPaintEnabled());
    mBoxesListAnimationDockWidget->updateSettingsForCurrentCanvas(canvas);
}

void MainWindow::setupToolBar() {
    mToolBar = new QToolBar(this);
    mToolBar->setMovable(false);


    mToolBar->setIconSize(QSize(24, 24));

    mToolBar->addSeparator();

    mMovePathMode = new ActionButton(
                ":/icons/draw_select.png",
                "F1", this);
    mMovePathMode->setCheckable(":/icons/draw_select_checked.png");
    mMovePathMode->setChecked(true);
    mToolBar->addWidget(mMovePathMode);

    mMovePointMode = new ActionButton(
                ":/icons/draw_node.png",
                "F2", this);
    mMovePointMode->setCheckable(":/icons/draw_node_checked.png");
    mToolBar->addWidget(mMovePointMode);

    mAddPointMode = new ActionButton(
                ":/icons/draw_pen.png",
                "F3", this);
    mAddPointMode->setCheckable(":/icons/draw_pen_checked.png");
    mToolBar->addWidget(mAddPointMode);

    mCircleMode = new ActionButton(
                ":/icons/draw_arc.png",
                "F4", this);
    mCircleMode->setCheckable(":/icons/draw_arc_checked.png");
    mToolBar->addWidget(mCircleMode);

    mToolBar->addSeparator();

    mRectangleMode = new ActionButton(
                ":/icons/draw_rect.png",
                "F5", this);
    mRectangleMode->setCheckable(":/icons/draw_rect_checked.png");
    mToolBar->addWidget(mRectangleMode);

    mTextMode = new ActionButton(
                ":/icons/draw_text.png",
                "F6", this);
    mTextMode->setCheckable(":/icons/draw_text_checked.png");
    mToolBar->addWidget(mTextMode);


    mToolBar->addSeparator();

    mActionConnectPoints = new ActionButton(
                ":/icons/node_join_segment.png",
                "CONNECT POINTS", this);
    mToolBar->addWidget(mActionConnectPoints);

    mActionDisconnectPoints = new ActionButton(
                ":/icons/node_delete_segment.png",
                "DISCONNECT POINTS", this);
    mToolBar->addWidget(mActionDisconnectPoints);

    mActionMergePoints = new ActionButton(
                ":/icons/node_join.png",
                "MERGE POINTS", this);
    mToolBar->addWidget(mActionMergePoints);
//
    mToolBar->addSeparator();

    mActionSymmetricPointCtrls = new ActionButton(
                ":/icons/node_symmetric.png",
                "SYMMETRIC POINTS", this);
    mToolBar->addWidget(mActionSymmetricPointCtrls);

    mActionSmoothPointCtrls = new ActionButton(
                ":/icons/node_smooth.png",
                "SMOOTH POINTS", this);
    mToolBar->addWidget(mActionSmoothPointCtrls);

    mActionCornerPointCtrls = new ActionButton(
                ":/icons/node_cusp.png",
                "CORNER POINTS", this);
    mToolBar->addWidget(mActionCornerPointCtrls);

//
    mToolBar->addSeparator();

    mActionLine = new ActionButton(
                ":/icons/node_line.png",
                "MAKE SEGMENT LINE", this);
    mToolBar->addWidget(mActionLine);

    mActionCurve = new ActionButton(
                ":/icons/node_curve.png",
                "MAKE SEGMENT CURVE", this);
    mToolBar->addWidget(mActionCurve);

    mToolBar->addSeparator();
//
    mFontWidget = new FontsWidget(this);
    mToolBar->addWidget(mFontWidget);

    mToolBar->addSeparator();

    QWidget *canvasComboWidget = new QWidget(this);
    canvasComboWidget->setAttribute(Qt::WA_TranslucentBackground);
    QHBoxLayout *canvasComboLayout = new QHBoxLayout();
    canvasComboLayout->setSpacing(0);
    canvasComboWidget->setLayout(canvasComboLayout);
    mCurrentCanvasComboBox = new QComboBox(mToolBar);
    mCurrentCanvasComboBox->setMinimumContentsLength(20);

    mNewCanvasButton = new QPushButton("+", mToolBar);
    mNewCanvasButton->setStyleSheet(
                "QPushButton {"
                    "border-top-right-radius: 0;"
                    "border-bottom-right-radius: 0;"
                    "border-right: 0px solid black;"
                    "background-color: rgb(51, 51, 51);"
                    "color: white;"
                    "font-weight: bold;"
                "}"
                "QPushButton:hover {"
                    "background-color: rgb(70, 70, 70);"
                "}");
    mCurrentCanvasComboBox->setStyleSheet(
                "border-top-left-radius: 0;"
                "border-bottom-left-radius: 0;");
    mNewCanvasButton->setFixedHeight(
                mCurrentCanvasComboBox->sizeHint().height() + 2);

    canvasComboLayout->addWidget(mNewCanvasButton);
    canvasComboLayout->addWidget(mCurrentCanvasComboBox);

    mToolBar->addWidget(canvasComboWidget);

    addToolBar(mToolBar);
}

void MainWindow::connectToolBarActions() {
    connect(mMovePathMode, SIGNAL(pressed()),
            mCanvasWidget, SLOT(setMovePathMode()) );
    connect(mMovePointMode, SIGNAL(pressed()),
            mCanvasWidget, SLOT(setMovePointMode()) );
    connect(mAddPointMode, SIGNAL(pressed()),
            mCanvasWidget, SLOT(setAddPointMode()) );
    connect(mCircleMode, SIGNAL(pressed()),
            mCanvasWidget, SLOT(setCircleMode()) );
    connect(mRectangleMode, SIGNAL(pressed()),
            mCanvasWidget, SLOT(setRectangleMode()) );
    connect(mTextMode, SIGNAL(pressed()),
            mCanvasWidget, SLOT(setTextMode()) );
    connect(mActionConnectPoints, SIGNAL(pressed()),
            mCanvasWidget, SLOT(connectPointsSlot()) );
    connect(mActionDisconnectPoints, SIGNAL(pressed()),
            mCanvasWidget, SLOT(disconnectPointsSlot()) );
    connect(mActionMergePoints, SIGNAL(pressed()),
            mCanvasWidget, SLOT(mergePointsSlot()) );
    connect(mActionSymmetricPointCtrls, SIGNAL(pressed()),
            mCanvasWidget, SLOT(makePointCtrlsSymmetric()) );
    connect(mActionSmoothPointCtrls, SIGNAL(pressed()),
            mCanvasWidget, SLOT(makePointCtrlsSmooth()) );
    connect(mActionCornerPointCtrls, SIGNAL(pressed()),
            mCanvasWidget, SLOT(makePointCtrlsCorner()) );
    connect(mActionLine, SIGNAL(pressed()),
            mCanvasWidget, SLOT(makeSegmentLine()) );
    connect(mActionCurve, SIGNAL(pressed()),
            mCanvasWidget, SLOT(makeSegmentCurve()) );
    connect(mCurrentCanvasComboBox, SIGNAL(editTextChanged(QString)),
            mCanvasWidget, SLOT(renameCurrentCanvas(QString)));
    connect(mCurrentCanvasComboBox, SIGNAL(currentIndexChanged(int)),
            mCanvasWidget, SLOT(setCurrentCanvas(int)));
    connect(mNewCanvasButton, SIGNAL(pressed()),
            this, SLOT(createNewCanvas()));
    connect(mFontWidget, SIGNAL(fontSizeChanged(qreal)),
            mCanvasWidget, SLOT(setFontSize(qreal)) );
    connect(mFontWidget, SIGNAL(fontFamilyAndStyleChanged(QString, QString)),
            mCanvasWidget, SLOT(setFontFamilyAndStyle(QString, QString)) );
}

MainWindow *MainWindow::getInstance()
{
    return mMainWindowInstance;
}
#include "newcanvasdialog.h"
void MainWindow::createNewCanvas() {
    QString defName = "Canvas " +
            QString::number(mCurrentCanvasComboBox->count());
    NewCanvasDialog dialog(defName, this);

    if(dialog.exec() == QDialog::Accepted) {
        Canvas *newCanvas = new Canvas(getFillStrokeSettings(),
                                       mCanvasWidget,
                                       dialog.getCanvasWidth(),
                                       dialog.getCanvasHeight());

        newCanvas->setName(dialog.getCanvasName());

        mCanvasWidget->addCanvasToListAndSetAsCurrent(newCanvas);

        disconnect(mCurrentCanvasComboBox, SIGNAL(currentIndexChanged(int)),
                mCanvasWidget, SLOT(setCurrentCanvas(int)));
        mCurrentCanvasComboBox->addItem(dialog.getCanvasName());
        mCurrentCanvasComboBox->setCurrentIndex(
                    mCurrentCanvasComboBox->count() - 1);
        connect(mCurrentCanvasComboBox, SIGNAL(currentIndexChanged(int)),
                mCanvasWidget, SLOT(setCurrentCanvas(int)));
    }
}

void MainWindow::createDetachedUndoRedoStack()
{
    mCurrentUndoRedoStack = new UndoRedoStack();
    mCurrentUndoRedoStack->setWindow(this);
    mDetachedUndoRedoStack = true;
}

void MainWindow::deleteDetachedUndoRedoStack()
{
    mDetachedUndoRedoStack = false;
    delete mCurrentUndoRedoStack;
    mCurrentUndoRedoStack = &mUndoRedoStack;
}

void MainWindow::updateCanvasModeButtonsChecked() {
    if(mCanvasWidget->hasNoCanvas()) return;
    const CanvasMode &currentMode =
            mCanvasWidget->getCurrentCanvas()->getCurrentCanvasMode();
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
        emit updateBoxPixmaps(box);
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
        if(mBoxesUpdateFinishedFunction != NULL) {
            (*this.*mBoxesUpdateFinishedFunction)();
        }
        //callUpdateSchedulers();
    } else {
        mLastUpdatedBox = mBoxesAwaitingUpdate.takeFirst();
        emit updateBoxPixmaps(mLastUpdatedBox);
    }
}

void MainWindow::playPreview()
{
    //mCanvas->clearPreview();
    mCanvas->updateRenderRect();
    mBoxesUpdateFinishedFunction = &MainWindow::nextPlayPreviewFrame;
    mSavedCurrentFrame = mCurrentFrame;

    mPreviewInterrupted = false;
    mCurrentRenderFrame = mSavedCurrentFrame;
    setCurrentFrame(mSavedCurrentFrame);
    mCanvas->setPreviewing(true);
    mCanvas->updateAllBoxes();
    if(mNoBoxesAwaitUpdate) {
        nextPlayPreviewFrame();
    }
}

void MainWindow::nextPlayPreviewFrame() {
    mCanvas->renderCurrentFrameToPreview();
    if(mCurrentRenderFrame >= mMaxFrame) {
        mBoxesListAnimationDockWidget->setCurrentFrame(mSavedCurrentFrame);
        mBoxesUpdateFinishedFunction = NULL;
        if(!mPreviewInterrupted) {
//            mSoundComposition->setFirstAndLastVideoFrame(0, 240);
//            mSoundComposition->generateData();

//            QAudioFormat desiredFormat;
//            desiredFormat.setChannelCount(2);
//            desiredFormat.setCodec("audio/x-raw");
//            desiredFormat.setSampleType(QAudioFormat::UnSignedInt);
//            desiredFormat.setSampleRate(48000);
//            desiredFormat.setSampleSize(16);

//            QAudioOutput *audio = new QAudioOutput(
//                                    QAudioDeviceInfo::defaultOutputDevice(),
//                                    desiredFormat,
//                                    this);
//            mSoundComposition->start();
//            audio->start(mSoundComposition);
            mCanvas->playPreview();
        }
    } else {
        mCurrentRenderFrame++;
        mBoxesListAnimationDockWidget->setCurrentFrame(mCurrentRenderFrame);
        if(mNoBoxesAwaitUpdate) {
            nextPlayPreviewFrame();
        }
    }
}

void MainWindow::nextSaveOutputFrame() {
    mCanvas->renderCurrentFrameToOutput(mOutputString);
    if(mCurrentRenderFrame >= mMaxFrame) {
        mBoxesListAnimationDockWidget->setCurrentFrame(mSavedCurrentFrame);
        mBoxesUpdateFinishedFunction = NULL;
    } else {
        mCurrentRenderFrame++;
        mBoxesListAnimationDockWidget->setCurrentFrame(mCurrentRenderFrame);
        if(mNoBoxesAwaitUpdate) {
            nextSaveOutputFrame();
        }
    }
}

void MainWindow::saveOutput(QString renderDest) {
    mOutputString = renderDest;
    mBoxesUpdateFinishedFunction = &MainWindow::nextSaveOutputFrame;
    mSavedCurrentFrame = mCurrentFrame;

    mCurrentRenderFrame = mMinFrame;
    mBoxesListAnimationDockWidget->setCurrentFrame(mMinFrame);
    if(mNoBoxesAwaitUpdate) {
        nextSaveOutputFrame();
    }
}

void MainWindow::previewFinished() {
    mBoxesListAnimationDockWidget->previewFinished();
}

void MainWindow::stopPreview() {
    mPreviewInterrupted = true;
    mCurrentRenderFrame = mMaxFrame;
    mCanvas->clearPreview();
    mCanvasWidget->repaint();
    previewFinished();
}

void MainWindow::setResolutionPercentId(int id)
{
    mCanvasWidget->setResolutionPercent(1. - id*0.25);
}

void MainWindow::renderOutput()
{
    RenderOutputWidget *dialog = new RenderOutputWidget(this);
    connect(dialog, SIGNAL(render(QString)), this, SLOT(saveOutput(QString)));
    dialog->exec();
}

UndoRedoStack *MainWindow::getUndoRedoStack()
{
    return mCurrentUndoRedoStack;
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
    if(!isEnabled()) {
        return;
    }
    mCurrentUndoRedoStack->finishSet();

    mKeysView->graphUpdateAfterKeysChangedIfNeeded();

    foreach(UpdateScheduler *sheduler, mUpdateSchedulers) {
        sheduler->update();
        delete sheduler;
    }

    mUpdateSchedulers.clear();
    mCanvasWidget->updatePivotIfNeeded();
    mCanvasWidget->repaint();
    mBoxListWidget->repaint();
    mObjectSettingsWidget->repaint();
    mKeysView->repaint();
    updateDisplayedFillStrokeSettingsIfNeeded();
    mFillStrokeSettings->repaint();

    mCurrentUndoRedoStack->startNewSet();
}

void MainWindow::setCurrentShapesMenuBox(BoundingBox *box) {
    if(mVectorShapesMenu == NULL) return;
    mVectorShapesMenu->setSelectedBoundingBox(box);
}

void MainWindow::setCurrentObjectSettingsWidgetBox(BoundingBox *box) {
    if(mObjectSettingsWidget == NULL) return;
    mObjectSettingsWidget->setBoundingBox(box);
}

void MainWindow::setCurrentBox(BoundingBox *box) {
    setCurrentShapesMenuBox(box);
    setCurrentObjectSettingsWidgetBox(box);
}

void MainWindow::updateDisplayedShapesInMenu() {
    if(mVectorShapesMenu == NULL) return;
    mVectorShapesMenu->updateDisplayedShapes();
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

void MainWindow::schedulePivotUpdate() {
    mCanvasWidget->schedulePivotUpdate();
}

KeysView *MainWindow::getKeysView()
{
    return mKeysView;
}

BoxesListWidget *MainWindow::getBoxesList()
{
    return mBoxesListAnimationDockWidget->getBoxesList();
}

void MainWindow::disableEventFilter() {
    mEventFilterDisabled = true;
}

void MainWindow::enableEventFilter() {
    mEventFilterDisabled = false;
}

void MainWindow::disable()
{
    mGrayOutWidget = new QWidget(this);
    mGrayOutWidget->setFixedSize(size());
    mGrayOutWidget->setStyleSheet("QWidget { background-color: rgb(0, 0, 0, 125) }");
    mGrayOutWidget->show();
    mGrayOutWidget->repaint();
}

void MainWindow::enable() {
    if(mGrayOutWidget == NULL) return;
    delete mGrayOutWidget;
    mGrayOutWidget = NULL;
}

int MainWindow::getCurrentFrame() {
    return mCurrentFrame;
}

bool MainWindow::isRecordingAllPoints() {
    return mAllPointsRecording;
}

int MainWindow::getMinFrame() {
    return mMinFrame;
}

int MainWindow::getMaxFrame() {
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
    mCanvasWidget->updateDisplayedFillStrokeSettings();
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
            mCurrentUndoRedoStack->redo();
        } else {
            mCurrentUndoRedoStack->undo();
        }
    } else if(isCtrlPressed() && event->key() == Qt::Key_S) {
        saveFile();
    } else if(isCtrlPressed() && event->key() == Qt::Key_O) {
        openFile();
    } else if(mCanvasWidget->processFilteredKeyEvent(event) ) {
    } else if(mKeysView->processFilteredKeyEvent(event)) {
    } else if(mBoxesListAnimationDockWidget->processFilteredKeyEvent(event) ) {
    } else {
        returnBool = false;
    }

    callUpdateSchedulers();
    return returnBool;
}

bool MainWindow::isEnabled() {
    return mGrayOutWidget == NULL;
}

void MainWindow::clearAll() {
    foreach(UpdateScheduler *sheduler, mUpdateSchedulers) {
        delete sheduler;
    }
    mUpdateSchedulers.clear();

    mUndoRedoStack.clearAll();
    mCanvas->clearAll();
    mFillStrokeSettings->clearAll();
    mObjectSettingsWidget->clearAll();
    mBoxListWidget->clearAll();
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
    QSqlQuery query;
    createTablesInSaveDatabase(&query);

    query.exec("BEGIN TRANSACTION");

    mFillStrokeSettings->saveGradientsToSqlIfPathSelected(&query);
    mCanvas->saveSelectedToSqlForCurrentBox(&query);

    query.exec("COMMIT TRANSACTION");
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
    QStringList importPaths = QFileDialog::getOpenFileNames(this,
        "Import File", "", "Files (*.av *.svg *.png *.jpg)");
    enableEventFilter();
    if(!importPaths.isEmpty()) {
        foreach(const QString &path, importPaths) {
            if(path.isEmpty()) continue;
            importFile(path, true);
        }
    }
}

void MainWindow::linkFile()
{
    disableEventFilter();
    QStringList importPaths = QFileDialog::getOpenFileNames(this,
        "Link File", "", "AniVect Files (*.av)");
    enableEventFilter();
    if(!importPaths.isEmpty()) {
        foreach(const QString &path, importPaths) {
            if(path.isEmpty()) continue;
            mCanvas->createLinkToFileWithPath(path);
        }
    }
}

void MainWindow::importAnimation() {
    disableEventFilter();
    QStringList importPaths = QFileDialog::getOpenFileNames(this,
        "Import Animation", "", "Images (*.png *.jpg)");
    enableEventFilter();
    if(!importPaths.isEmpty()) {
        mCanvas->createAnimationBoxForPaths(importPaths);
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

void MainWindow::setCurrentFrameForAllWidgets(int frame)
{
    mBoxesListAnimationDockWidget->setCurrentFrame(frame);
}

void MainWindow::importFile(QString path, bool loadInBox) {
    if(mCanvasWidget->hasNoCanvas()) return;
    disable();

    QFile file(path);
    if(!file.exists()) {
        return;
    }
    QString extension = path.split(".").last();
    if(extension == "svg") {
        loadSVGFile(path, mCanvasWidget->getCurrentCanvas());
    } else if(extension == "av") {
        QSqlDatabase db = QSqlDatabase::database();//not dbConnection
        db.setDatabaseName(path);
        db.open();

        mFillStrokeSettings->loadAllGradientsFromSql();
        mCanvasWidget->getCurrentCanvas()->loadAllBoxesFromSql(loadInBox);

        db.close();
    } else if(extension == "png" ||
              extension == "jpg") {
        mCanvasWidget->getCurrentCanvas()->createImageBox(path);
    }
    enable();

    callUpdateSchedulers();
}

void MainWindow::loadFile(QString path) {
    clearAll();

    importFile(path, false);

    mUndoRedoStack.clearAll();
    setFileChangedSinceSaving(false);
    setCurrentFrame(0);
}

void MainWindow::createTablesInSaveDatabase(QSqlQuery *query) {
    query->exec("CREATE TABLE qrealanimator "
               "(id INTEGER PRIMARY KEY,"
               "currentvalue REAL )");

    query->exec("CREATE TABLE qrealkey "
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

    query->exec("CREATE TABLE transformanimator "
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

    query->exec("CREATE TABLE qpointfanimator "
               "(id INTEGER PRIMARY KEY, "
               "xanimatorid INTEGER, "
               "yanimatorid INTEGER, "
               "FOREIGN KEY(xanimatorid) REFERENCES qrealanimator(id), "
               "FOREIGN KEY(yanimatorid) REFERENCES qrealanimator(id) )");


    query->exec("CREATE TABLE coloranimator "
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

    query->exec("CREATE TABLE gradient "
               "(id INTEGER PRIMARY KEY)");
    query->exec("CREATE TABLE gradientcolor "
               "(colorid INTEGER, "
               "gradientid INTEGER, "
               "positioningradient INTEGER, "
               "FOREIGN KEY(colorid) REFERENCES coloranimator(id), "
               "FOREIGN KEY(gradientid) REFERENCES gradient(id) )");
    query->exec("CREATE TABLE paintsettings "
               "(id INTEGER PRIMARY KEY, "
               "painttype INTEGER, "
               "colorid INTEGER, "
               "gradientid INTEGER, "
               "FOREIGN KEY(colorid) REFERENCES coloranimator(id), "
               "FOREIGN KEY(gradientid) REFERENCES gradient(id) )");
    query->exec("CREATE TABLE strokesettings "
               "(id INTEGER PRIMARY KEY, "
               "linewidthanimatorid INTEGER, "
               "capstyle INTEGER, "
               "joinstyle INTEGER, "
               "paintsettingsid INTEGER, "
               "FOREIGN KEY(linewidthanimatorid) REFERENCES qrealanimator(id), "
               "FOREIGN KEY(paintsettingsid) REFERENCES paintsettings(id) )");

    query->exec("CREATE TABLE boundingbox "
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
    query->exec("CREATE TABLE boxesgroup "
               "(id INTEGER PRIMARY KEY, "
               "boundingboxid INTEGER, "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");
    query->exec("CREATE TABLE pathbox "
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
    query->exec("CREATE TABLE movablepoint "
               "(id INTEGER PRIMARY KEY, "
               "posanimatorid INTEGER, "
               "FOREIGN KEY(posanimatorid) REFERENCES qrealanimator(id) )");
    query->exec("CREATE TABLE pathpoint "
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
    query->exec("CREATE TABLE circle "
               "(id INTEGER PRIMARY KEY, "
               "boundingboxid INTEGER, "
               "horizontalradiuspointid INTEGER, "
               "verticalradiuspointid INTEGER, "
               "FOREIGN KEY(horizontalradiuspointid) REFERENCES movablepoint(id), "
               "FOREIGN KEY(verticalradiuspointid) REFERENCES movablepoint(id), "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");

    query->exec("CREATE TABLE rectangle "
               "(id INTEGER PRIMARY KEY, "
               "boundingboxid INTEGER, "
               "topleftpointid INTEGER, "
               "bottomrightpointid INTEGER, "
               "radiuspointid INTEGER, "
               "FOREIGN KEY(topleftpointid) REFERENCES movablepoint(id), "
               "FOREIGN KEY(bottomrightpointid) REFERENCES movablepoint(id), "
               "FOREIGN KEY(radiuspointid) REFERENCES movablepoint(id), "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");

    query->exec("CREATE TABLE textbox "
               "(id INTEGER PRIMARY KEY, "
               "boundingboxid INTEGER, "
               "text TEXT, "
               "fontfamily TEXT, "
               "fontstyle TEXT, "
               "fontsize REAL, "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");

    query->exec("CREATE TABLE pixmapeffect "
                "(id INTEGER PRIMARY KEY, "
                "boundingboxid INTEGER, "
                "type INTEGER, "
                "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");

    query->exec("CREATE TABLE blureffect "
                "(id INTEGER PRIMARY KEY, "
                "pixmapeffectid INTEGER, "
                "radiusid INTEGER, "
                "FOREIGN KEY(pixmapeffectid) REFERENCES pixmapeffect(id), "
                "FOREIGN KEY(radiusid) REFERENCES qrealanimator(id) )");
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

    QSqlQuery query;
    createTablesInSaveDatabase(&query);

    query.exec("BEGIN TRANSACTION");

    mFillStrokeSettings->saveGradientsToQuery(&query);
    mCanvas->saveToSql(&query);

    query.exec("COMMIT TRANSACTION");
    db.close();

    enable();
}
