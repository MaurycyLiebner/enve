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
#include "svgimporter.h"
#include "canvaswindow.h"
#include "BoxesList/boxscrollwidget.h"
#include "clipboardcontainer.h"
#include "BoxesList/OptimalScrollArea/scrollarea.h"
#include "BoxesList/boxscrollwidgetvisiblepart.h"
#include "RenderWidget/renderwidget.h"
#include "actionbutton.h"
#include "fontswidget.h"
#include "global.h"

#include <QAudioOutput>
#include "Sound/soundcomposition.h"
#include "Sound/singlesound.h"
#include "BoxesList/boxsinglewidget.h"
#include "memoryhandler.h"

extern "C" {
    #include <libavformat/avformat.h>
}

MainWindow *MainWindow::mMainWindowInstance;

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    processKeyEvent(event);
}

int FONT_HEIGHT;
int MIN_WIDGET_HEIGHT;
int KEY_RECT_SIZE;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    FONT_HEIGHT = QApplication::fontMetrics().height();
    MIN_WIDGET_HEIGHT = FONT_HEIGHT*4/3;
    KEY_RECT_SIZE = MIN_WIDGET_HEIGHT*3/5;
    av_register_all();
    QFile file("/home/ailuropoda/Dev/AniVect/stylesheet.qss");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setStyleSheet(file.readAll());
        file.close();
    }
    //setMouseTracking(true);
    //mSoundComposition = new SoundComposition();
//    mSoundComposition->addSound(
//                new SingleSound("/home/ailuropoda/.Qt_pro/build-AniVect-Desktop_Qt_5_7_0_GCC_64bit-Debug/lektor.wav"));
    BoxSingleWidget::loadStaticPixmaps();
    setupToolBar();

    for(int i = 0; i < ClipboardContainerType::CCT_COUNT; i++) {
        mClipboardContainers << NULL;
    }

    mCurrentUndoRedoStack = &mUndoRedoStack;

    mMemoryHandler = new MemoryHandler(this);

    mMainWindowInstance = this;
    //int nThreads = QThread::idealThreadCount();

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

    mCanvasWindow = new CanvasWindow(this);
    connect(mMemoryHandler, SIGNAL(allMemoryUsed()),
            mCanvasWindow, SLOT(outOfMemory()));

    mBoxesListAnimationDockWidget = new BoxesListAnimationDockWidget(this);
    connect(mCanvasWindow, SIGNAL(changeCurrentFrame(int)),
            mBoxesListAnimationDockWidget, SLOT(setCurrentFrame(int)));
    connect(mCanvasWindow, SIGNAL(changeFrameRange(int,int)),
            mBoxesListAnimationDockWidget, SLOT(setMinMaxFrame(int,int)));
    mBottomDock->setWidget(mBoxesListAnimationDockWidget);

    mFillStrokeSettings->setCanvasWindowPtr(mCanvasWindow);

    setupMenuBar();

    setCentralWidget(mCanvasWindow->getCanvasWidget());

    showMaximized();

    mLeftDock = new QDockWidget(this);
    mLeftDock->setFeatures(mLeftDock->features().setFlag(
                               QDockWidget::DockWidgetClosable, false));
    mLeftDock->setMinimumWidth(MIN_WIDGET_HEIGHT*10);

    mObjectSettingsScrollArea = new ScrollArea(this);
    mObjectSettingsWidget = new BoxScrollWidget(mObjectSettingsScrollArea);
    mObjectSettingsScrollArea->setWidget(mObjectSettingsWidget);
    mObjectSettingsWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_Selected);
    mObjectSettingsWidget->getVisiblePartWidget()->
            setCurrentTarget(NULL, SWT_CurrentGroup);

    connect(mObjectSettingsScrollArea->verticalScrollBar(),
            SIGNAL(valueChanged(int)),
            mObjectSettingsWidget, SLOT(changeVisibleTop(int)));
    connect(mObjectSettingsScrollArea, SIGNAL(heightChanged(int)),
            mObjectSettingsWidget, SLOT(changeVisibleHeight(int)));
    connect(mObjectSettingsScrollArea, SIGNAL(widthChanged(int)),
            mObjectSettingsWidget, SLOT(setWidth(int)));

    mObjectSettingsScrollArea->verticalScrollBar()->setSingleStep(
                MIN_WIDGET_HEIGHT);

    mLeftDock->setWidget(mObjectSettingsScrollArea);
    addDockWidget(Qt::LeftDockWidgetArea, mLeftDock);

//    mCanvasWindow->SWT_getAbstractionForWidget(
//                mBoxListWidget->getVisiblePartWidget());

//    Canvas *canvas = new Canvas(mFillStrokeSettings, mCanvasWindow);
//    canvas->setName("Canvas 0");
//    mCanvasWindow->addCanvasToListAndSetAsCurrent(canvas);
//    mCanvas = mCanvasWindow->getCurrentCanvas();
//    mCurrentCanvasComboBox->addItem(mCanvas->getName());

    connectToolBarActions();

    this->setMouseTracking(true);
    centralWidget()->setMouseTracking(true);

    createNewCanvas();

    mEventFilterDisabled = false;
}

MainWindow::~MainWindow() {
//    mPaintControlerThread->terminate();
//    mPaintControlerThread->quit();
}

void MainWindow::setupMenuBar() {
    mMenuBar = new QMenuBar(this);

    mFileMenu = mMenuBar->addMenu("File");
    mFileMenu->addAction("New...", this, SLOT(newFile()));
    mFileMenu->addAction("Open...", this, SLOT(openFile()));
    mFileMenu->addSeparator();
    mFileMenu->addAction("Link...", this, SLOT(linkFile()));
    mFileMenu->addAction("Import File...", mCanvasWindow, SLOT(importFile()));
    mFileMenu->addAction("Import Image Sequence...", this, SLOT(importImageSequence()));
    mFileMenu->addSeparator();
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
    mObjectMenu->addAction("Raise", mCanvasWindow,
                           SLOT(raiseAction()));
    mObjectMenu->addAction("Lower", mCanvasWindow,
                           SLOT(lowerAction()));
    mObjectMenu->addAction("Rasie to Top", mCanvasWindow,
                           SLOT(raiseToTopAction()));
    mObjectMenu->addAction("Lower to Bottom", mCanvasWindow,
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

    mPathMenu->addAction("Object to Path", mCanvasWindow,
                         SLOT(objectsToPathAction()));
    mPathMenu->addAction("Stroke to Path", mCanvasWindow,
                         SLOT(strokeToPathAction()));
    mPathMenu->addSeparator();
    mPathMenu->addAction("Union", mCanvasWindow,
                         SLOT(pathsUnionAction()));
    mPathMenu->addAction("Difference", mCanvasWindow,
                         SLOT(pathsDifferenceAction()));
    mPathMenu->addAction("Intersection", mCanvasWindow,
                         SLOT(pathsIntersectionAction()));
    mPathMenu->addAction("Exclusion", mCanvasWindow,
                         SLOT(pathsExclusionAction()));
    mPathMenu->addAction("Division", mCanvasWindow,
                         SLOT(pathsDivisionAction()));
//    mPathMenu->addAction("Cut Path", mCanvas,
//                         SLOT(pathsCutAction()));
    mPathMenu->addSeparator();
    mPathMenu->addAction("Combine", mCanvasWindow,
                         SLOT(pathsCombineAction()));
    mPathMenu->addAction("Break Apart", mCanvasWindow,
                         SLOT(pathsBreakApartAction()));

    mEffectsMenu = mMenuBar->addMenu("Effects");

    mEffectsMenu->addAction("Blur");

    mViewMenu = mMenuBar->addMenu("View");

    mActionEffectsPaintEnabled = mViewMenu->addAction("Effects");
    mActionEffectsPaintEnabled->setCheckable(true);
    mActionEffectsPaintEnabled->setChecked(true);
    mActionEffectsPaintEnabled->setShortcut(QKeySequence(Qt::Key_E));

    mRenderMenu = mMenuBar->addMenu("Render");
    mRenderMenu->addAction("Render",
                           this, SLOT(addCanvasToRenderQue()));

    setMenuBar(mMenuBar);
//

    connect(mActionEffectsPaintEnabled, SIGNAL(toggled(bool)),
            mCanvasWindow, SLOT(setEffectsPaintEnabled(bool)));
}

void MainWindow::addCanvasToRenderQue() {
    if(mCanvasWindow->hasNoCanvas()) return;
    mBoxesListAnimationDockWidget->getRenderWidget()->
    createNewRenderInstanceWidgetForCanvas(mCanvasWindow->getCurrentCanvas());
}

void MainWindow::updateSettingsForCurrentCanvas() {
    if(mCanvasWindow->hasNoCanvas()) {
        mObjectSettingsWidget->setMainTarget(NULL);
        return;
    }
    Canvas *canvas = mCanvasWindow->getCurrentCanvas();
    mActionEffectsPaintEnabled->setChecked(canvas->effectsPaintEnabled());
    mBoxesListAnimationDockWidget->updateSettingsForCurrentCanvas(canvas);
    mObjectSettingsWidget->setMainTarget(
                canvas->getCurrentBoxesGroup());
}

void MainWindow::replaceClipboard(ClipboardContainer *container) {
    ClipboardContainer *clipboardContainer = mClipboardContainers.at(
                                                container->getType());
    if(clipboardContainer != NULL) {
        delete clipboardContainer;
    }
    mClipboardContainers.replace(container->getType(),
                                 container);
}

ClipboardContainer *MainWindow::getClipboardContainer(
        const ClipboardContainerType &type) {
    return mClipboardContainers.at(type);
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

    mParticleBoxMode = new ActionButton(
                ":/icons/draw_particle_box.png",
                "F7", this);
    mParticleBoxMode->setCheckable(":/icons/draw_particle_box_checked.png");
    mToolBar->addWidget(mParticleBoxMode);

    mParticleEmitterMode = new ActionButton(
                ":/icons/draw_particle_emitter.png",
                "F8", this);
    mParticleEmitterMode->setCheckable(
                ":/icons/draw_particle_emitter_checked.png");
    mToolBar->addWidget(mParticleEmitterMode);

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
                mCurrentCanvasComboBox->sizeHint().height());

    canvasComboLayout->addWidget(mNewCanvasButton);
    canvasComboLayout->addWidget(mCurrentCanvasComboBox);

    mToolBar->addWidget(canvasComboWidget);

    addToolBar(mToolBar);
}

void MainWindow::connectToolBarActions() {
    connect(mMovePathMode, SIGNAL(pressed()),
            mCanvasWindow, SLOT(setMovePathMode()) );
    connect(mMovePointMode, SIGNAL(pressed()),
            mCanvasWindow, SLOT(setMovePointMode()) );
    connect(mAddPointMode, SIGNAL(pressed()),
            mCanvasWindow, SLOT(setAddPointMode()) );
    connect(mCircleMode, SIGNAL(pressed()),
            mCanvasWindow, SLOT(setCircleMode()) );
    connect(mRectangleMode, SIGNAL(pressed()),
            mCanvasWindow, SLOT(setRectangleMode()) );
    connect(mTextMode, SIGNAL(pressed()),
            mCanvasWindow, SLOT(setTextMode()) );
    connect(mParticleBoxMode, SIGNAL(pressed()),
            mCanvasWindow, SLOT(setParticleBoxMode()) );
    connect(mParticleEmitterMode, SIGNAL(pressed()),
            mCanvasWindow, SLOT(setParticleEmitterMode()));
    connect(mActionConnectPoints, SIGNAL(pressed()),
            mCanvasWindow, SLOT(connectPointsSlot()) );
    connect(mActionDisconnectPoints, SIGNAL(pressed()),
            mCanvasWindow, SLOT(disconnectPointsSlot()) );
    connect(mActionMergePoints, SIGNAL(pressed()),
            mCanvasWindow, SLOT(mergePointsSlot()) );
    connect(mActionSymmetricPointCtrls, SIGNAL(pressed()),
            mCanvasWindow, SLOT(makePointCtrlsSymmetric()) );
    connect(mActionSmoothPointCtrls, SIGNAL(pressed()),
            mCanvasWindow, SLOT(makePointCtrlsSmooth()) );
    connect(mActionCornerPointCtrls, SIGNAL(pressed()),
            mCanvasWindow, SLOT(makePointCtrlsCorner()) );
    connect(mActionLine, SIGNAL(pressed()),
            mCanvasWindow, SLOT(makeSegmentLine()) );
    connect(mActionCurve, SIGNAL(pressed()),
            mCanvasWindow, SLOT(makeSegmentCurve()) );
    connect(mCurrentCanvasComboBox, SIGNAL(editTextChanged(QString)),
            mCanvasWindow, SLOT(renameCurrentCanvas(QString)));
    connect(mCurrentCanvasComboBox, SIGNAL(currentIndexChanged(int)),
            mCanvasWindow, SLOT(setCurrentCanvas(int)));
    connect(mNewCanvasButton, SIGNAL(pressed()),
            this, SLOT(createNewCanvas()));
    connect(mFontWidget, SIGNAL(fontSizeChanged(qreal)),
            mCanvasWindow, SLOT(setFontSize(qreal)) );
    connect(mFontWidget, SIGNAL(fontFamilyAndStyleChanged(QString, QString)),
            mCanvasWindow, SLOT(setFontFamilyAndStyle(QString, QString)) );
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
                                       mCanvasWindow,
                                       dialog.getCanvasWidth(),
                                       dialog.getCanvasHeight(),
                                       dialog.getCanvasFrameCount());

        newCanvas->setName(dialog.getCanvasName());

        addCanvas(newCanvas);
    }
}

void MainWindow::addCanvas(Canvas *newCanvas) {
    mCanvasWindow->addCanvasToListAndSetAsCurrent(newCanvas);

    disconnect(mCurrentCanvasComboBox, SIGNAL(currentIndexChanged(int)),
            mCanvasWindow, SLOT(setCurrentCanvas(int)));
    mCurrentCanvasComboBox->addItem(newCanvas->getName());
    mCurrentCanvasComboBox->setCurrentIndex(
                mCurrentCanvasComboBox->count() - 1);
    connect(mCurrentCanvasComboBox, SIGNAL(currentIndexChanged(int)),
            mCanvasWindow, SLOT(setCurrentCanvas(int)));
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
    if(mCanvasWindow->hasNoCanvas()) return;
    const CanvasMode &currentMode =
            mCanvasWindow->getCurrentCanvas()->getCurrentCanvasMode();
    mMovePathMode->setChecked(currentMode == MOVE_PATH);
    mMovePointMode->setChecked(currentMode == MOVE_POINT);
    mAddPointMode->setChecked(currentMode == ADD_POINT);
    mCircleMode->setChecked(currentMode == ADD_CIRCLE);
    mRectangleMode->setChecked(currentMode == ADD_RECTANGLE);
    mTextMode->setChecked(currentMode == ADD_TEXT);
    mParticleBoxMode->setChecked(currentMode == ADD_PARTICLE_BOX);
    mParticleEmitterMode->setChecked(currentMode == ADD_PARTICLE_EMITTER);
}

//void MainWindow::addBoxAwaitingUpdate(BoundingBox *box)
//{
//    if(mNoBoxesAwaitUpdate) {
//        mNoBoxesAwaitUpdate = false;
//        mLastUpdatedBox = box;
//        emit updateBoxPixmaps(box);
//    } else {
//        mBoxesAwaitingUpdate << box;
//    }
//}

//void MainWindow::sendNextBoxForUpdate()
//{
//    if(mLastUpdatedBox != NULL) {
//        mLastUpdatedBox->setAwaitingUpdate(false);
//        if(mLastUpdatedBox->shouldRedoUpdate()) {
//            mLastUpdatedBox->setRedoUpdateToFalse();
//            mLastUpdatedBox->awaitUpdate();
//        }
//    }
//    if(mBoxesAwaitingUpdate.isEmpty()) {
//        mNoBoxesAwaitUpdate = true;
//        mLastUpdatedBox = NULL;
//        callUpdateSchedulers();
//        if(mBoxesUpdateFinishedFunction != NULL) {
//            (*this.*mBoxesUpdateFinishedFunction)();
//        }
//        //callUpdateSchedulers();
//    } else {
//        mLastUpdatedBox = mBoxesAwaitingUpdate.takeFirst();
//        emit updateBoxPixmaps(mLastUpdatedBox);
//    }
//}

//void MainWindow::playPreview()
//{
//    //mCanvas->clearPreview();
//    mCanvas->updateRenderRect();
//    mBoxesUpdateFinishedFunction = &MainWindow::nextPlayPreviewFrame;
//    mSavedCurrentFrame = mCurrentFrame;

//    mRendering = true;
//    mPreviewInterrupted = false;
//    mCurrentRenderFrame = mSavedCurrentFrame;
//    setCurrentFrame(mSavedCurrentFrame);
//    mCanvas->setPreviewing(true);
//    mCanvas->updateAllBoxes();
//    if(mNoBoxesAwaitUpdate) {
//        nextPlayPreviewFrame();
//    }
//}

//void MainWindow::nextPlayPreviewFrame() {
//    mCanvas->renderCurrentFrameToPreview();
//    if(mCurrentRenderFrame >= mMaxFrame || mPreviewInterrupted) {
//        mRendering = false;
//        mBoxesListAnimationDockWidget->setCurrentFrame(mSavedCurrentFrame);
//        mBoxesUpdateFinishedFunction = NULL;
//            mCanvas->playPreview();
//    } else {
//        mCurrentRenderFrame++;
//        mBoxesListAnimationDockWidget->setCurrentFrame(mCurrentRenderFrame);
//        if(mNoBoxesAwaitUpdate) {
//            nextPlayPreviewFrame();
//        }
//    }
//}

//void MainWindow::nextSaveOutputFrame() {
//    mCanvas->renderCurrentFrameToOutput(mOutputString);
//    if(mCurrentRenderFrame >= mMaxFrame) {
//        mBoxesListAnimationDockWidget->setCurrentFrame(mSavedCurrentFrame);
//        mBoxesUpdateFinishedFunction = NULL;
//    } else {
//        mCurrentRenderFrame++;
//        mBoxesListAnimationDockWidget->setCurrentFrame(mCurrentRenderFrame);
//        if(mNoBoxesAwaitUpdate) {
//            nextSaveOutputFrame();
//        }
//    }
//}

//void MainWindow::saveOutput(QString renderDest) {
//    mOutputString = renderDest;
//    mBoxesUpdateFinishedFunction = &MainWindow::nextSaveOutputFrame;
//    mSavedCurrentFrame = mCurrentFrame;

//    mCurrentRenderFrame = mMinFrame;
//    mBoxesListAnimationDockWidget->setCurrentFrame(mMinFrame);
//    if(mNoBoxesAwaitUpdate) {
//        nextSaveOutputFrame();
//    }
//}

void MainWindow::previewFinished() {
    mBoxesListAnimationDockWidget->previewFinished();
}

void MainWindow::previewBeingPlayed() {
    mBoxesListAnimationDockWidget->previewBeingPlayed();
}

void MainWindow::previewBeingRendered() {
    mBoxesListAnimationDockWidget->previewBeingRendered();
}

void MainWindow::previewPaused() {
    mBoxesListAnimationDockWidget->previewPaused();
}

//void MainWindow::stopPreview() {
//    mPreviewInterrupted = true;
//    if(!mRendering) {
//        mCurrentRenderFrame = mMaxFrame;
//        mCanvas->clearPreview();
//        mCanvasWindow->repaint();
//        previewFinished();
//    }
//}

void MainWindow::setResolutionFractionValue(const qreal &value) {
    mCanvasWindow->setResolutionFraction(value);
}

UndoRedoStack *MainWindow::getUndoRedoStack() {
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

void MainWindow::callUpdateSchedulers() {
    if(!isEnabled()) {
        return;
    }
    mCurrentUndoRedoStack->finishSet();

    //mKeysView->graphUpdateAfterKeysChangedIfNeeded();

    Q_FOREACH(UpdateScheduler *sheduler, mUpdateSchedulers) {
        sheduler->update();
        delete sheduler;
    }

    ScrollWidgetVisiblePart::callAllInstanceUpdaters();
    mUpdateSchedulers.clear();
    mCanvasWindow->updateHoveredElements();
    mCanvasWindow->updatePivotIfNeeded();
    mCanvasWindow->repaint();
    mObjectSettingsWidget->update();
    //mKeysView->repaint();
    mBoxesListAnimationDockWidget->update();
    updateDisplayedFillStrokeSettingsIfNeeded();
    mFillStrokeSettings->update();
    emit updateAll();

    mCurrentUndoRedoStack->startNewSet();
}

void MainWindow::setCurrentBox(BoundingBox *box) {
    if(box == NULL) {
        mFillStrokeSettings->setCurrentSettings(NULL,
                                                NULL);
    } else {
        mFillStrokeSettings->setCurrentSettings(box->getFillSettings(),
                                                box->getStrokeSettings());
    }
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
    mCanvasWindow->schedulePivotUpdate();
}

BoxScrollWidget *MainWindow::getObjectSettingsList() {
    return mObjectSettingsWidget;
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
    mGrayOutWidget->setStyleSheet(
                "QWidget { background-color: rgb(0, 0, 0, 125) }");
    mGrayOutWidget->show();
    mGrayOutWidget->update();
}

void MainWindow::enable() {
    if(mGrayOutWidget == NULL) return;
    delete mGrayOutWidget;
    mGrayOutWidget = NULL;
}

int MainWindow::getCurrentFrame() {
    return mCanvasWindow->getCurrentFrame();
}

bool MainWindow::isRecordingAllPoints() {
    return mAllPointsRecording;
}

int MainWindow::getFrameCount() {
    return mCanvasWindow->getMaxFrame();
}
#include "Colors/ColorWidgets/gradientwidget.h"
void MainWindow::setCurrentFrame(int frame) {
    mFillStrokeSettings->getGradientWidget()->updateAfterFrameChanged(frame);
    mCanvasWindow->updateAfterFrameChanged(frame);

    callUpdateSchedulers();
}

void MainWindow::setAllPointsRecord(bool allPointsRecord) {
    mAllPointsRecording = allPointsRecord;
}

void MainWindow::newFile()
{
    if(askForSaving()) {
        closeProject();
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e) {
    if(mEventFilterDisabled) {
        return QMainWindow::eventFilter(obj, e);
    }
    if (e->type() == QEvent::KeyPress) {
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
    } else if(obj == mCanvasWindow->getCanvasWidget()) {
        if(e->type() == QEvent::Drop) {
            mCanvasWindow->dropEvent((QDropEvent*)e);
        } else if(e->type() == QEvent::DragEnter) {
            mCanvasWindow->dragEnterEvent((QDragEnterEvent*)e);
        } else if(e->type() == QEvent::FocusIn) {
            mCanvasWindow->getCanvasWidget();
        }
    }
    return QMainWindow::eventFilter(obj, e);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if(!askForSaving() ) {
        e->ignore();
    }
}

void MainWindow::updateDisplayedFillStrokeSettings() {
    mCanvasWindow->updateDisplayedFillStrokeSettings();
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
    } else if(mCanvasWindow->processFilteredKeyEvent(event) ) {
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
    Q_FOREACH(UpdateScheduler *sheduler, mUpdateSchedulers) {
        delete sheduler;
    }
    mUpdateSchedulers.clear();

    mUndoRedoStack.clearAll();
    mObjectSettingsWidget->setMainTarget(NULL);
    mBoxesListAnimationDockWidget->clearAll();
    mCurrentCanvasComboBox->clear();
    mCanvasWindow->clearAll();
    mFillStrokeSettings->clearAll();
    //mBoxListWidget->clearAll();
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

void MainWindow::openFile() {
    if(askForSaving()) {
        disableEventFilter();
        QString openPath = QFileDialog::getOpenFileName(this,
            "Open File", "", "AniVect Files (*.av)");
        enableEventFilter();
        if(!openPath.isEmpty()) {
            setCurrentPath(openPath);
            loadAVFile(mCurrentFilePath);
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

void MainWindow::linkFile() {
    disableEventFilter();
    QStringList importPaths = QFileDialog::getOpenFileNames(this,
        "Link File", "", "AniVect Files (*.av)");
    enableEventFilter();
    if(!importPaths.isEmpty()) {
        Q_FOREACH(const QString &path, importPaths) {
            if(path.isEmpty()) continue;
            mCanvasWindow->createLinkToFileWithPath(path);
        }
    }
}

void MainWindow::importImageSequence() {
    disableEventFilter();
    QStringList importPaths = QFileDialog::getOpenFileNames(this,
        "Import Image Sequence", "", "Images (*.png *.jpg)");
    enableEventFilter();
    if(!importPaths.isEmpty()) {
        mCanvasWindow->createAnimationBoxForPaths(importPaths);
    }
    callUpdateSchedulers();
}

//void MainWindow::importVideo() {
//    disableEventFilter();
//    QStringList importPaths = QFileDialog::getOpenFileNames(this,
//        "Import Video", "", "Video (*.mp4 *.mov *.avi)");
//    enableEventFilter();
//    Q_FOREACH(const QString &path, importPaths) {
//        mCanvasWindow->createVideoForPath(path);
//    }
//    callUpdateSchedulers();
//}

void MainWindow::revert()
{
    loadAVFile(mCurrentFilePath);
    setFileChangedSinceSaving(false);
}

void MainWindow::setCurrentFrameForAllWidgets(int frame)
{
    mBoxesListAnimationDockWidget->setCurrentFrame(frame);
}

void MainWindow::loadAVFile(QString path) {
    clearAll();

    QSqlDatabase db = QSqlDatabase::database();//not dbConnection
    db.setDatabaseName(path);
    db.open();

    loadAllGradientsFromSql();

    mCanvasWindow->loadCanvasesFromSql();

    clearLoadedGradientsList();
    db.close();

    mUndoRedoStack.clearAll();
    setFileChangedSinceSaving(false);
    setCurrentFrame(0);
}


Gradient *MainWindow::getLoadedGradientBySqlId(const int &id) {
    Q_FOREACH(Gradient *gradient, mLoadedGradientsList) {
        if(gradient->getSqlId() == id) {
            return gradient;
        }
    }
    return NULL;
}

void MainWindow::loadAllGradientsFromSql() {
    QSqlQuery query;
    QString queryStr = QString("SELECT * FROM gradient");
    if(query.exec(queryStr) ) {
        int idId = query.record().indexOf("id");
        while(query.next() ) {
            mLoadedGradientsList <<
                new Gradient(query.value(idId).toInt());
        }
    } else {
        qDebug() << "Could not load gradients";
    }
}

void MainWindow::clearLoadedGradientsList() {
    mLoadedGradientsList.clear();
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

    query->exec("CREATE TABLE gradientpoints "
                "(id INTEGER PRIMARY KEY, "
                "endpointid INTEGER, "
                "startpointid INTEGER, "
                "FOREIGN KEY(endpointid) REFERENCES qpointfanimator(id), "
                "FOREIGN KEY(startpointid) REFERENCES qpointfanimator(id) )");

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

    query->exec("CREATE TABLE canvas "
               "(id INTEGER PRIMARY KEY, "
               "boundingboxid INTEGER, "
               "width INTEGER, "
               "height INTEGER, "
               "framecount INTEGER, "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");

    query->exec("CREATE TABLE pathbox "
               "(id INTEGER PRIMARY KEY, "
               "fillgradientpointsid INTEGER, "
               "fillgradientendid INTEGER, "
               "strokegradientpointsid INTEGER, "
               "strokegradientendid INTEGER, "
               "boundingboxid INTEGER, "
               "fillsettingsid INTEGER, "
               "strokesettingsid INTEGER, "
               "FOREIGN KEY(fillgradientpointsid) REFERENCES gradientpoints(id), "
               "FOREIGN KEY(strokegradientpointsid) REFERENCES gradientpoints(id), "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id), "
               "FOREIGN KEY(fillsettingsid) REFERENCES paintsettings(id), "
               "FOREIGN KEY(strokesettingsid) REFERENCES strokesettings(id) )");

    query->exec("CREATE TABLE pathpoint "
               "(id INTEGER PRIMARY KEY, "
               "isfirst BOOLEAN, "
               "isendpoint BOOLEAN, "
               "qpointfanimatorid INTEGER, "
               "startctrlptid INTEGER, "
               "endctrlptid INTEGER, "
               "boundingboxid INTEGER, "
               "ctrlsmode INTEGER, "
               "startpointenabled BOOLEAN, "
               "endpointenabled BOOLEAN, "
               "FOREIGN KEY(qpointfanimatorid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(startctrlptid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(endctrlptid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");

    query->exec("CREATE TABLE circle "
               "(id INTEGER PRIMARY KEY, "
               "boundingboxid INTEGER, "
               "horizontalradiuspointid INTEGER, "
               "verticalradiuspointid INTEGER, "
               "FOREIGN KEY(horizontalradiuspointid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(verticalradiuspointid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");

    query->exec("CREATE TABLE rectangle "
               "(id INTEGER PRIMARY KEY, "
               "boundingboxid INTEGER, "
               "topleftpointid INTEGER, "
               "bottomrightpointid INTEGER, "
               "radiuspointid INTEGER, "
               "FOREIGN KEY(topleftpointid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(bottomrightpointid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(radiuspointid) REFERENCES qpointfanimator(id), "
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

    query->exec("CREATE TABLE shadoweffect "
                "(id INTEGER PRIMARY KEY, "
                "pixmapeffectid INTEGER, "
                "blurradiusid INTEGER, "
                "colorid INTEGER, "
                "opacityid INTEGER, "
                "translationid INTEGER, "
                "FOREIGN KEY(pixmapeffectid) REFERENCES pixmapeffect(id), "
                "FOREIGN KEY(blurradiusid) REFERENCES qrealanimator(id), "
                "FOREIGN KEY(colorid) REFERENCES coloranimator(id), "
                "FOREIGN KEY(opacityid) REFERENCES qrealanimator(id), "
                "FOREIGN KEY(translationid) REFERENCES qpointfanimator(id) )");

    query->exec("CREATE TABLE lineseffect "
                "(id INTEGER PRIMARY KEY, "
                "pixmapeffectid INTEGER, "
                "distanceid INTEGER, "
                "widthid INTEGER, "
                "FOREIGN KEY(pixmapeffectid) REFERENCES pixmapeffect(id), "
                "FOREIGN KEY(distanceid) REFERENCES qrealanimator(id), "
                "FOREIGN KEY(widthid) REFERENCES qrealanimator(id) )");

    query->exec("CREATE TABLE circleseffect "
                "(id INTEGER PRIMARY KEY, "
                "pixmapeffectid INTEGER, "
                "distanceid INTEGER, "
                "radiusid INTEGER, "
                "FOREIGN KEY(pixmapeffectid) REFERENCES pixmapeffect(id), "
                "FOREIGN KEY(distanceid) REFERENCES qrealanimator(id), "
                "FOREIGN KEY(radiusid) REFERENCES qrealanimator(id) )");

    query->exec("CREATE TABLE swirleffect "
                "(id INTEGER PRIMARY KEY, "
                "pixmapeffectid INTEGER, "
                "degreesid INTEGER, "
                "FOREIGN KEY(pixmapeffectid) REFERENCES pixmapeffect(id), "
                "FOREIGN KEY(degreesid) REFERENCES qrealanimator(id) )");

    query->exec("CREATE TABLE oileffect "
                "(id INTEGER PRIMARY KEY, "
                "pixmapeffectid INTEGER, "
                "radiusid INTEGER, "
                "FOREIGN KEY(pixmapeffectid) REFERENCES pixmapeffect(id), "
                "FOREIGN KEY(radiusid) REFERENCES qrealanimator(id) )");

    query->exec("CREATE TABLE implodeeffect "
                "(id INTEGER PRIMARY KEY, "
                "pixmapeffectid INTEGER, "
                "factorid INTEGER, "
                "FOREIGN KEY(pixmapeffectid) REFERENCES pixmapeffect(id), "
                "FOREIGN KEY(factorid) REFERENCES qrealanimator(id) )");

    query->exec("CREATE TABLE desaturateeffect "
                "(id INTEGER PRIMARY KEY, "
                "pixmapeffectid INTEGER, "
                "influenceid INTEGER, "
                "FOREIGN KEY(pixmapeffectid) REFERENCES pixmapeffect(id), "
                "FOREIGN KEY(influenceid) REFERENCES qrealanimator(id) )");

    query->exec("CREATE TABLE alphamatteeffect "
                "(id INTEGER PRIMARY KEY, "
                "pixmapeffectid INTEGER, "
                "influenceid INTEGER, "
                "boundingboxid INTEGER, "
                "inverted BOOLEAN, "
                "FOREIGN KEY(pixmapeffectid) REFERENCES pixmapeffect(id), "
                "FOREIGN KEY(influenceid) REFERENCES qrealanimator(id), "
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

    QSqlQuery query;
    createTablesInSaveDatabase(&query);

    query.exec("BEGIN TRANSACTION");

    mFillStrokeSettings->saveGradientsToQuery(&query);
    mCanvasWindow->saveToSql(&query);

    query.exec("COMMIT TRANSACTION");
    db.close();

    enable();
}
