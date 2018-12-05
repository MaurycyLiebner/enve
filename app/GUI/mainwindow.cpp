#include "mainwindow.h"
#include "canvas.h"
#include <QKeyEvent>
#include <QApplication>
#include <QDebug>
#include <QStatusBar>
#include "usagewidget.h"
#include <QToolBar>
#include "GUI/ColorWidgets/colorsettingswidget.h"
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include "boxeslistanimationdockwidget.h"
#include "paintcontroler.h"
#include "qdoubleslider.h"
#include "svgimporter.h"
#include "canvaswindow.h"
#include "GUI/BoxesList/boxscrollwidget.h"
#include "clipboardcontainer.h"
#include "GUI/BoxesList/OptimalScrollArea/scrollarea.h"
#include "GUI/BoxesList/boxscrollwidgetvisiblepart.h"
#include "GUI/RenderWidgets/renderwidget.h"
#include "actionbutton.h"
#include "fontswidget.h"
#include "global.h"
#include "filesourcescache.h"
#include "filesourcelist.h"
#include "videoencoder.h"
#include "fillstrokesettings.h"

#include <QAudioOutput>
#include "Sound/soundcomposition.h"
#include "Sound/singlesound.h"
#include "GUI/BoxesList/boxsinglewidget.h"
#include "memoryhandler.h"
 #include "GUI/BrushWidgets/brushselectionwidget.h"
extern "C" {
    #include <libavformat/avformat.h>
}

MainWindow *MainWindow::mMainWindowInstance;

void MainWindow::keyPressEvent(QKeyEvent *event) {
    processKeyEvent(event);
}

int FONT_HEIGHT;
int MIN_WIDGET_HEIGHT;
int KEY_RECT_SIZE;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    mVideoEncoder = SPtrCreate(VideoEncoder)();
    FONT_HEIGHT = QApplication::fontMetrics().height();
    MIN_WIDGET_HEIGHT = FONT_HEIGHT*4/3;
    KEY_RECT_SIZE = MIN_WIDGET_HEIGHT*3/5;
    av_register_all();
    QFile file(":/styles/stylesheet.qss");
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
    setupStatusBar();

//    for(int i = 0; i < ClipboardContainerType::CCT_COUNT; i++) {
//        mClipboardContainers << nullptr;
//    }

    mMemoryHandler = new MemoryHandler(this);

    mMainWindowInstance = this;
    //int nThreads = QThread::idealThreadCount();

    setCurrentPath("");

    mRightDock = new QDockWidget(this);
    mFillStrokeSettings = new FillStrokeSettingsWidget(this);
    mRightDock->setWidget(mFillStrokeSettings);
    mRightDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    mRightDock->setTitleBarWidget(new QWidget());
    addDockWidget(Qt::RightDockWidgetArea, mRightDock);
    mRightDock->setMinimumWidth(MIN_WIDGET_HEIGHT*10);
    mRightDock->setMaximumWidth(MIN_WIDGET_HEIGHT*20);

    mBottomDock = new QDockWidget(this);

    mBottomDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
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

    mBrushSettingsDock = new QDockWidget(this);
    mBrushSettingsDock->setFeatures(mBrushSettingsDock->features().setFlag(
                               QDockWidget::DockWidgetClosable, false));
    mBrushSettingsDock->setMinimumWidth(MIN_WIDGET_HEIGHT*10);
    mBrushSettingsDock->setMaximumWidth(MIN_WIDGET_HEIGHT*20);
    QLabel *brushDockLabel = new QLabel("Brush Settings", this);
    brushDockLabel->setStyleSheet(
                "QLabel {"
                    "border-top: 0;"
                    "border-bottom: 1px solid black;"
                    "color: black;"
                    "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
                    "stop:0 lightgray, stop:1 darkgray);"
                "}");
    brushDockLabel->setAlignment(Qt::AlignCenter);
    mBrushSettingsDock->setTitleBarWidget(brushDockLabel);

    mBrushSelectionWidget = new BrushSelectionWidget(this);
//    connect(mBrushSettingsWidget, SIGNAL(brushSelected(const Brush*)),
//            mCanvasWindow, SLOT(setCurrentBrush(const Brush*)));
//    connect(mBrushSettingsWidget,
//            SIGNAL(brushReplaced(const Brush*,const Brush*)),
//            mCanvasWindow,
//            SLOT(replaceBrush(const Brush*,const Brush*)));

    mBrushSettingsDock->setWidget(mBrushSelectionWidget);

    addDockWidget(Qt::LeftDockWidgetArea, mBrushSettingsDock);
    mBrushSettingsDock->hide();

    mFillStrokeSettings->setCanvasWindowPtr(mCanvasWindow);

    mLeftDock = new QDockWidget(this);
    mLeftDock->setFeatures(mLeftDock->features().setFlag(
                               QDockWidget::DockWidgetClosable, false));
    mLeftDock->setMinimumWidth(MIN_WIDGET_HEIGHT*10);
    mLeftDock->setMaximumWidth(MIN_WIDGET_HEIGHT*20);
    QLabel *leftDockLabel = new QLabel("Current Object", this);
    leftDockLabel->setStyleSheet(
                "QLabel {"
                    "border-top: 0;"
                    "border-bottom: 1px solid black;"
                    "color: black;"
                    "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
                    "stop:0 lightgray, stop:1 darkgray);"
                "}");
    leftDockLabel->setAlignment(Qt::AlignCenter);
    mLeftDock->setTitleBarWidget(leftDockLabel);

    mObjectSettingsScrollArea = new ScrollArea(this);
    mObjectSettingsWidget = new BoxScrollWidget(mObjectSettingsScrollArea);
    mObjectSettingsScrollArea->setWidget(mObjectSettingsWidget);
    mObjectSettingsWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_Selected);
    mObjectSettingsWidget->getVisiblePartWidget()->
            setCurrentTarget(nullptr, SWT_CurrentGroup);

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

    mLeftDock2 = new QDockWidget(this);
    mLeftDock2->setFeatures(mLeftDock2->features().setFlag(
                               QDockWidget::DockWidgetClosable, false));
    mLeftDock2->setMinimumWidth(MIN_WIDGET_HEIGHT*10);
    mLeftDock2->setMaximumWidth(MIN_WIDGET_HEIGHT*20);

    mLeftDock2->setWidget(new FileSourceList(this));
    addDockWidget(Qt::LeftDockWidgetArea, mLeftDock2);

    QLabel *leftDock2Label = new QLabel("Files", this);
    leftDock2Label->setStyleSheet(
                "QLabel {"
                    "border-top: 0;"
                    "border-bottom: 1px solid black;"
                    "color: black;"
                    "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
                    "stop:0 lightgray, stop:1 darkgray);"
                "}");
    leftDock2Label->setAlignment(Qt::AlignCenter);
    mLeftDock2->setTitleBarWidget(leftDock2Label);


//    mCanvasWindow->SWT_getAbstractionForWidget(
//                mBoxListWidget->getVisiblePartWidget());

//    Canvas *canvas = new Canvas(mFillStrokeSettings, mCanvasWindow);
//    canvas->setName("Canvas 0");
//    mCanvasWindow->addCanvasToListAndSetAsCurrent(canvas);
//    mCanvas = mCanvasWindow->getCurrentCanvas();
//    mCurrentCanvasComboBox->addItem(mCanvas->getName());

    setupMenuBar();

    connectToolBarActions();

    setCentralWidget(mCanvasWindow->getCanvasWidget());

    showMaximized();

    this->setMouseTracking(true);
    centralWidget()->setMouseTracking(true);

    createNewCanvas();

    mEventFilterDisabled = false;

    QApplication::instance()->installEventFilter(this);
}

MainWindow::~MainWindow() {
//    mPaintControlerThread->terminate();
//    mPaintControlerThread->quit();
    replaceClipboard(nullptr);
    BoxSingleWidget::clearStaticPixmaps();
}

#include "noshortcutaction.h"
void MainWindow::setupMenuBar() {
    mMenuBar = new QMenuBar(this);

    mFileMenu = mMenuBar->addMenu("File");
    mFileMenu->addAction("New...",
                         this, SLOT(newFile()),
                         Qt::CTRL + Qt::Key_N);
    mFileMenu->addAction("Open...",
                         this, SLOT(openFile()),
                         Qt::CTRL + Qt::Key_O);
    mFileMenu->addSeparator();
    mFileMenu->addAction("Link...",
                         this, SLOT(linkFile()),
                         Qt::CTRL + Qt::Key_L);
    mFileMenu->addAction("Import File...",
                         mCanvasWindow, SLOT(importFile()),
                         Qt::CTRL + Qt::Key_I);
    mFileMenu->addAction("Import Image Sequence...",
                         this, SLOT(importImageSequence()));
    mFileMenu->addSeparator();
    mFileMenu->addAction("Revert", this, SLOT(revert()));
    mFileMenu->addSeparator();
    mFileMenu->addAction("Save",
                         this, SLOT(saveFile()),
                         Qt::CTRL + Qt::Key_S);
    mFileMenu->addAction("Save As...",
                         this, SLOT(saveFileAs()),
                         Qt::CTRL + Qt::SHIFT + Qt::Key_S);
    mFileMenu->addAction("Save Backup",
                         this, SLOT(saveBackup()));
    mFileMenu->addSeparator();
    mFileMenu->addAction("Close",
                         this, SLOT(closeProject()));
    mFileMenu->addSeparator();
    mFileMenu->addAction("Exit",
                         this, SLOT(close()));

    mEditMenu = mMenuBar->addMenu("Edit");

    mEditMenu->addAction("Undo",
                         this, SLOT(undo()),
                         Qt::CTRL + Qt::Key_Z);
    mEditMenu->addAction("Redo",
                         this, SLOT(redo()),
                         Qt::CTRL + Qt::SHIFT + Qt::Key_Z);
    mEditMenu->addAction("Undo History...");
    mEditMenu->addSeparator();
    mEditMenu->addAction(new NoShortcutAction("Cut",
                         mCanvasWindow, SLOT(cutAction()),
                         Qt::CTRL + Qt::Key_X, mEditMenu));
    mEditMenu->addAction(new NoShortcutAction("Copy",
                         mCanvasWindow, SLOT(copyAction()),
                         Qt::CTRL + Qt::Key_C, mEditMenu));
    mEditMenu->addAction(new NoShortcutAction("Paste",
                         mCanvasWindow, SLOT(pasteAction()),
                         Qt::CTRL + Qt::Key_V, mEditMenu));
    mEditMenu->addSeparator();
    mEditMenu->addAction(new NoShortcutAction("Duplicate",
                         mCanvasWindow, SLOT(duplicateAction()),
                         Qt::CTRL + Qt::Key_D, mEditMenu));
    mEditMenu->addSeparator();
    mEditMenu->addAction(new NoShortcutAction("Delete",
                         mCanvasWindow, SLOT(deleteAction()),
                         Qt::Key_Delete, mEditMenu));
    mEditMenu->addSeparator();
    mEditMenu->addAction(new NoShortcutAction("Select All",
                         mCanvasWindow, SLOT(selectAllAction()),
                         Qt::Key_A, mEditMenu));
    mEditMenu->addAction(new NoShortcutAction("Invert Selection",
                                              mCanvasWindow, SLOT(invertSelectionAction()),
                                              Qt::Key_I, mEditMenu));
    mEditMenu->addAction(new NoShortcutAction("Clear Selection",
                         mCanvasWindow, SLOT(clearSelectionAction()),
                         Qt::ALT + Qt::Key_A, mEditMenu));

//    mSelectSameMenu = mEditMenu->addMenu("Select Same");
//    mSelectSameMenu->addAction("Fill and Stroke");
//    mSelectSameMenu->addAction("Fill Color");
//    mSelectSameMenu->addAction("Stroke Color");
//    mSelectSameMenu->addAction("Stroke Style");
//    mSelectSameMenu->addAction("Object Type");

    mObjectMenu = mMenuBar->addMenu("Object");

    mObjectMenu->addSeparator();
    mObjectMenu->addAction("Raise",
                           mCanvasWindow, SLOT(raiseAction()),
                           Qt::Key_PageUp);
    mObjectMenu->addAction("Lower",
                           mCanvasWindow, SLOT(lowerAction()),
                           Qt::Key_PageDown);
    mObjectMenu->addAction("Rasie to Top",
                           mCanvasWindow, SLOT(raiseToTopAction()),
                           Qt::Key_Home);
    mObjectMenu->addAction("Lower to Bottom",
                           mCanvasWindow, SLOT(lowerToBottomAction()))->
            setShortcut(Qt::Key_End);
    mObjectMenu->addSeparator();
    mObjectMenu->addAction("Rotate 90° CW",
                           mCanvasWindow, SLOT(rotate90CWAction()));
    mObjectMenu->addAction("Rotate 90° CCW",
                           mCanvasWindow, SLOT(rotate90CCWAction()));
    mObjectMenu->addAction("Flip Horizontal", mCanvasWindow,
                           SLOT(flipHorizontalAction()), Qt::Key_H);
    mObjectMenu->addAction("Flip Vertical", mCanvasWindow,
                           SLOT(flipVerticalAction()), Qt::Key_V);
    mObjectMenu->addSeparator();
    mObjectMenu->addAction("Group", mCanvasWindow,
                           SLOT(groupSelectedBoxes()),
                           Qt::CTRL + Qt::Key_G);
    mObjectMenu->addAction("Ungroup", mCanvasWindow,
                           SLOT(ungroupSelectedBoxes()),
                           Qt::CTRL + Qt::SHIFT + Qt::Key_G);

    mPathMenu = mMenuBar->addMenu("Path");

    mPathMenu->addAction("Object to Path", mCanvasWindow,
                         SLOT(objectsToPathAction()));
    mPathMenu->addAction("Stroke to Path", mCanvasWindow,
                         SLOT(strokeToPathAction()));
    mPathMenu->addSeparator();
    mPathMenu->addAction("Union", mCanvasWindow,
                         SLOT(pathsUnionAction()))->
            setShortcut(Qt::CTRL + Qt::Key_Plus);
    mPathMenu->addAction("Difference", mCanvasWindow,
                         SLOT(pathsDifferenceAction()))->
            setShortcut(Qt::CTRL + Qt::Key_Minus);
    mPathMenu->addAction("Intersection", mCanvasWindow,
                         SLOT(pathsIntersectionAction()))->
            setShortcut(Qt::CTRL + Qt::Key_Asterisk);
    mPathMenu->addAction("Exclusion", mCanvasWindow,
                         SLOT(pathsExclusionAction()))->
            setShortcut(Qt::CTRL + Qt::Key_AsciiCircum);
    mPathMenu->addAction("Division", mCanvasWindow,
                         SLOT(pathsDivisionAction()))->
            setShortcut(Qt::CTRL + Qt::Key_Slash);
//    mPathMenu->addAction("Cut Path", mCanvas,
//                         SLOT(pathsCutAction()));
    mPathMenu->addSeparator();
    mPathMenu->addAction("Combine", mCanvasWindow,
                         SLOT(pathsCombineAction()))->
            setShortcut(Qt::CTRL + Qt::Key_K);
    mPathMenu->addAction("Break Apart", mCanvasWindow,
                         SLOT(pathsBreakApartAction()))->
            setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_K);

//    mEffectsMenu = mMenuBar->addMenu("Effects");

//    mEffectsMenu->addAction("Blur");

    mViewMenu = mMenuBar->addMenu("View");

    mClipViewToCanvas = mViewMenu->addAction("Clip To Canvas");
    mClipViewToCanvas->setCheckable(true);
    mClipViewToCanvas->setChecked(true);
    mClipViewToCanvas->setShortcut(QKeySequence(Qt::Key_C));
    connect(mClipViewToCanvas, SIGNAL(toggled(bool)),
            mCanvasWindow, SLOT(setClipToCanvas(bool)));

    mRasterEffectsVisible = mViewMenu->addAction("Raster Effects");
    mRasterEffectsVisible->setCheckable(true);
    mRasterEffectsVisible->setChecked(true);
    connect(mRasterEffectsVisible, SIGNAL(toggled(bool)),
            mCanvasWindow, SLOT(setRasterEffectsVisible(bool)));

    mPathEffectsVisible = mViewMenu->addAction("Path Effects");
    mPathEffectsVisible->setCheckable(true);
    mPathEffectsVisible->setChecked(true);
    connect(mPathEffectsVisible, SIGNAL(toggled(bool)),
            mCanvasWindow, SLOT(setPathEffectsVisible(bool)));


    mPanelsMenu = mViewMenu->addMenu("Docks");

    mCurrentObjectDock = mPanelsMenu->addAction("Current Object");
    mCurrentObjectDock->setCheckable(true);
    mCurrentObjectDock->setChecked(true);
    mCurrentObjectDock->setShortcut(QKeySequence(Qt::Key_O));

    connect(mCurrentObjectDock, SIGNAL(toggled(bool)),
            mLeftDock, SLOT(setVisible(bool)));

    mFilesDock = mPanelsMenu->addAction("Files");
    mFilesDock->setCheckable(true);
    mFilesDock->setChecked(true);
    mFilesDock->setShortcut(QKeySequence(Qt::Key_F));

    connect(mFilesDock, SIGNAL(toggled(bool)),
            mLeftDock2, SLOT(setVisible(bool)));

    mObjectsAndAnimationsDock = mPanelsMenu->addAction("Objects and Animations");
    mObjectsAndAnimationsDock->setCheckable(true);
    mObjectsAndAnimationsDock->setChecked(true);
    mObjectsAndAnimationsDock->setShortcut(QKeySequence(Qt::Key_T));

    connect(mObjectsAndAnimationsDock, SIGNAL(toggled(bool)),
            mBottomDock,
            SLOT(setVisible(bool)));

    mFillAndStrokeSettingsDock = mPanelsMenu->addAction("Fill and Stroke");
    mFillAndStrokeSettingsDock->setCheckable(true);
    mFillAndStrokeSettingsDock->setChecked(true);
    mFillAndStrokeSettingsDock->setShortcut(QKeySequence(Qt::Key_E));

    connect(mFillAndStrokeSettingsDock, SIGNAL(toggled(bool)),
            mRightDock,
            SLOT(setVisible(bool)));

    mBrushSettingsDockAction = mPanelsMenu->addAction("Brush Settings");
    mBrushSettingsDockAction->setCheckable(true);
    mBrushSettingsDockAction->setChecked(false);
    mBrushSettingsDockAction->setShortcut(QKeySequence(Qt::Key_B));

    connect(mBrushSettingsDockAction, SIGNAL(toggled(bool)),
            mBrushSettingsDock,
            SLOT(setVisible(bool)));

    mRenderMenu = mMenuBar->addMenu("Render");
    mRenderMenu->addAction("Render",
                           this, SLOT(addCanvasToRenderQue()));

    setMenuBar(mMenuBar);
//

}

void MainWindow::addCanvasToRenderQue() {
    if(mCanvasWindow->hasNoCanvas()) return;
    mBoxesListAnimationDockWidget->getRenderWidget()->
    createNewRenderInstanceWidgetForCanvas(mCanvasWindow->getCurrentCanvas());
}

void MainWindow::updateSettingsForCurrentCanvas() {
    if(mCanvasWindow->hasNoCanvas()) {
        mObjectSettingsWidget->setMainTarget(nullptr);
        //mBrushSettingsWidget->setCurrentBrush(nullptr);
        mBoxesListAnimationDockWidget->updateSettingsForCurrentCanvas(nullptr);
        return;
    }
    Canvas* canvas = mCanvasWindow->getCurrentCanvas();
    mClipViewToCanvas->setChecked(canvas->clipToCanvas());
    mRasterEffectsVisible->setChecked(canvas->getRasterEffectsVisible());
    mPathEffectsVisible->setChecked(canvas->getPathEffectsVisible());
    mBoxesListAnimationDockWidget->updateSettingsForCurrentCanvas(canvas);
    mObjectSettingsWidget->setMainTarget(canvas->getCurrentBoxesGroup());
//    mBrushSettingsWidget->setCurrentBrush(canvas->getCurrentBrush());
    updateDisplayedFillStrokeSettings();
}

void MainWindow::replaceClipboard(const stdsptr<ClipboardContainer>& container) {
    if(mClipboardContainer != nullptr) {
        BoundingBox::clearLoadedBoxes();
    }
    mClipboardContainer = container;
}

ClipboardContainer *MainWindow::getClipboardContainer(
        const ClipboardContainerType &type) {
    if(mClipboardContainer == nullptr) return nullptr;
    if(type == mClipboardContainer->getType()) {
        return mClipboardContainer.get();
    }
    return nullptr;
}

#include <QSpacerItem>
void MainWindow::setupStatusBar() {
#ifdef QT_DEBUG
    mStatusBar = new QStatusBar(this);
    setStatusBar(mStatusBar);
    mUsageWidget = new UsageWidget(mStatusBar);
    mStatusBar->addWidget(mUsageWidget);
#endif
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

    mPickPaintSettingsMode = new ActionButton(
                ":/icons/draw_dropper.png",
                "F5", this);
    mPickPaintSettingsMode->setCheckable(":/icons/draw_dropper_checked.png");
    mToolBar->addWidget(mPickPaintSettingsMode);

    mToolBar->addSeparator();

    mCircleMode = new ActionButton(
                ":/icons/draw_arc.png",
                "F6", this);
    mCircleMode->setCheckable(":/icons/draw_arc_checked.png");
    mToolBar->addWidget(mCircleMode);

    mRectangleMode = new ActionButton(
                ":/icons/draw_rect.png",
                "F7", this);
    mRectangleMode->setCheckable(":/icons/draw_rect_checked.png");
    mToolBar->addWidget(mRectangleMode);

    mTextMode = new ActionButton(
                ":/icons/draw_text.png",
                "F8", this);
    mTextMode->setCheckable(":/icons/draw_text_checked.png");
    mToolBar->addWidget(mTextMode);

    mToolBar->addSeparator();

    mParticleBoxMode = new ActionButton(
                ":/icons/draw_particle_box.png",
                "F9", this);
    mParticleBoxMode->setCheckable(":/icons/draw_particle_box_checked.png");
    mToolBar->addWidget(mParticleBoxMode);

    mParticleEmitterMode = new ActionButton(
                ":/icons/draw_particle_emitter.png",
                "F10", this);
    mParticleEmitterMode->setCheckable(
                ":/icons/draw_particle_emitter_checked.png");
    mToolBar->addWidget(mParticleEmitterMode);

    mPaintBoxMode = new ActionButton(
                ":/icons/add_paint_box.png",
                "F11", this);
    mPaintBoxMode->setCheckable(
                ":/icons/add_paint_box_checked.png");
    mToolBar->addWidget(mPaintBoxMode);

    mPaintMode = new ActionButton(
                ":/icons/paint_mode.png",
                "F12", this);
    mPaintMode->setCheckable(
                ":/icons/paint_mode_checked.png");
    mToolBar->addWidget(mPaintMode);

    //mToolBar->addSeparator();
    mToolBar->widgetForAction(mToolBar->addAction("     "))->
            setObjectName("inactiveToolButton");
    //mToolBar->addSeparator();

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

    //mToolBar->addSeparator();
    mToolBar->widgetForAction(mToolBar->addAction("     "))->
            setObjectName("inactiveToolButton");
    //mToolBar->addSeparator();
//
    mFontWidget = new FontsWidget(this);
    mToolBar->addWidget(mFontWidget);

    //mToolBar->addSeparator();
    mToolBar->widgetForAction(mToolBar->addAction("     "))->
            setObjectName("inactiveToolButton");
    //mToolBar->addSeparator();
    QWidget *canvasComboWidget = new QWidget(this);
    canvasComboWidget->setAttribute(Qt::WA_TranslucentBackground);
    QHBoxLayout *canvasComboLayout = new QHBoxLayout();
    canvasComboLayout->setSpacing(0);
    canvasComboWidget->setLayout(canvasComboLayout);
    mCurrentCanvasComboBox = new QComboBox(mToolBar);
    mCurrentCanvasComboBox->setMinimumContentsLength(20);

    mNewCanvasButton = new QPushButton("+", mToolBar);
    mNewCanvasButton->setObjectName("addCanvasButton");
    mCurrentCanvasComboBox->setObjectName("currentCanvasComboBox");
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
    connect(mPickPaintSettingsMode, SIGNAL(pressed()),
            mCanvasWindow, SLOT(setPickPaintSettingsMode()) );
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
    connect(mPaintBoxMode, SIGNAL(pressed()),
            mCanvasWindow, SLOT(setPaintBoxMode()));
    connect(mPaintMode, SIGNAL(pressed()),
            mCanvasWindow, SLOT(setPaintMode()));
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

MainWindow *MainWindow::getInstance() {
    return mMainWindowInstance;
}

#include "newcanvasdialog.h"
void MainWindow::createNewCanvas() {
    QString defName = "Scene " +
            QString::number(mCurrentCanvasComboBox->count());
    qsptr<Canvas> newCanvas =
            SPtrCreate(Canvas)(mCanvasWindow, 1920, 1080, 200);
    newCanvas->setName(defName);
    CanvasSettingsDialog *dialog =
            new CanvasSettingsDialog(newCanvas.get(), this);

    int dialogRet = dialog->exec();
    if(dialogRet == QDialog::Accepted) {
        dialog->applySettingsToCanvas(newCanvas.get());

        addCanvas(newCanvas);
    }
    delete dialog;
}

void MainWindow::addCanvas(const qsptr<Canvas>& newCanvas) {
    mCanvasWindow->addCanvasToListAndSetAsCurrent(newCanvas);

    disconnect(mCurrentCanvasComboBox, SIGNAL(currentIndexChanged(int)),
            mCanvasWindow, SLOT(setCurrentCanvas(int)));
    mCurrentCanvasComboBox->addItem(newCanvas->getName());
    mCurrentCanvasComboBox->setCurrentIndex(
                mCurrentCanvasComboBox->count() - 1);
    connect(newCanvas.get(), &Canvas::canvasNameChanged,
            this, &MainWindow::canvasNameChanged);

    connect(mCurrentCanvasComboBox, SIGNAL(currentIndexChanged(int)),
            mCanvasWindow, SLOT(setCurrentCanvas(int)));
    newCanvas->fitCanvasToSize();
}

void MainWindow::canvasNameChanged(Canvas *canvas,
                                   const QString &name) {
    const QList<qsptr<Canvas>> &canvasList = mCanvasWindow->getCanvasList();
    int idT = 0;
    foreach(const qsptr<Canvas> &canvasPtr, canvasList) {
        if(canvasPtr == canvas) {
            break;
        }
        idT++;
    }

    if(idT < 0 || idT >= canvasList.count()) return;
    mCurrentCanvasComboBox->setItemText(idT, name);
}

void MainWindow::updateCanvasModeButtonsChecked() {
    if(mCanvasWindow->hasNoCanvas()) return;
    const CanvasMode &currentMode =
            mCanvasWindow->getCurrentCanvas()->getCurrentCanvasMode();
    mMovePathMode->setChecked(currentMode == MOVE_PATH);
    mMovePointMode->setChecked(currentMode == MOVE_POINT);
    mAddPointMode->setChecked(currentMode == ADD_POINT);
    mPickPaintSettingsMode->setChecked(currentMode == PICK_PAINT_SETTINGS);
    mCircleMode->setChecked(currentMode == ADD_CIRCLE);
    mRectangleMode->setChecked(currentMode == ADD_RECTANGLE);
    mTextMode->setChecked(currentMode == ADD_TEXT);
    mParticleBoxMode->setChecked(currentMode == ADD_PARTICLE_BOX);
    mParticleEmitterMode->setChecked(currentMode == ADD_PARTICLE_EMITTER);
    mPaintBoxMode->setChecked(currentMode == ADD_PAINT_BOX);
    mPaintMode->setChecked(currentMode == PAINT_MODE);
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
//    if(mLastUpdatedBox != nullptr) {
//        mLastUpdatedBox->setAwaitingUpdate(false);
//        if(mLastUpdatedBox->shouldRedoUpdate()) {
//            mLastUpdatedBox->setRedoUpdateToFalse();
//            mLastUpdatedBox->awaitUpdate();
//        }
//    }
//    if(mBoxesAwaitingUpdate.isEmpty()) {
//        mNoBoxesAwaitUpdate = true;
//        mLastUpdatedBox = nullptr;
//        callUpdateSchedulers();
//        if(mBoxesUpdateFinishedFunction != nullptr) {
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
//        mBoxesUpdateFinishedFunction = nullptr;
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
//        mBoxesUpdateFinishedFunction = nullptr;
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

void MainWindow::incBrushRadius() {
//    mBrushSettingsWidget->incBrushRadius();
}

void MainWindow::decBrushRadius() {
//    mBrushSettingsWidget->decBrushRadius();
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

void MainWindow::addUpdateScheduler(const stdsptr<_ScheduledExecutor>& scheduler) {
    mUpdateSchedulers.append(scheduler);
}

void MainWindow::addFileUpdateScheduler(const stdsptr<_ScheduledExecutor>& scheduler) {
    mCanvasWindow->addFileUpdatableAwaitingUpdate(scheduler);
}

bool MainWindow::isShiftPressed() {
    return QApplication::keyboardModifiers() & Qt::ShiftModifier;
}

bool MainWindow::isCtrlPressed() {
    return QApplication::keyboardModifiers() & Qt::ControlModifier;
}

bool MainWindow::isAltPressed() {
    return QApplication::keyboardModifiers() & Qt::AltModifier;
}

void MainWindow::finishUndoRedoSet() {
    if(mCurrentUndoRedoStack == nullptr) return;
    mCurrentUndoRedoStack->finishSet();
    mCurrentUndoRedoStack->startNewSet();
}

Brush *MainWindow::getCurrentBrush() {
//    return mBrushSettingsWidget->getCurrentBrush();
    return nullptr;
}

void MainWindow::callUpdateSchedulers() {
    if(!isEnabled()) {
        return;
    }
//    mCurrentUndoRedoStack->finishSet();

    //mKeysView->graphUpdateAfterKeysChangedIfNeeded();

    //if(mCanvasWindow->shouldProcessAwaitingSchedulers()) {
        mCanvasWindow->processSchedulers();
        foreach(const stdsptr<_ScheduledExecutor> &updatable,
                mUpdateSchedulers) {
            if(!updatable->isAwaitingUpdate()) {
                updatable->schedulerProccessed();
            }
            mCanvasWindow->addUpdatableAwaitingUpdate(updatable);
        }
        mUpdateSchedulers.clear();
    //}

    mCanvasWindow->updateHoveredElements();
    mCanvasWindow->updatePivotIfNeeded();
    mCanvasWindow->requestUpdate();
    ScrollWidgetVisiblePart::callAllInstanceUpdaters();
    mObjectSettingsWidget->update();
    //mKeysView->repaint();
    mBoxesListAnimationDockWidget->update();
    updateDisplayedFillStrokeSettingsIfNeeded();
    mFillStrokeSettings->update();
    emit updateAll();

//    mCurrentUndoRedoStack->startNewSet();
}
#include "Boxes/textbox.h"
void MainWindow::setCurrentBox(BoundingBox *box) {
    if(box == nullptr) {
        mFillStrokeSettings->setCurrentSettings(nullptr,
                                                nullptr);
    } else {
        mFillStrokeSettings->setCurrentSettings(box->getFillSettings(),
                                                box->getStrokeSettings());
        if(box->SWT_isTextBox()) {
            TextBox *txtBox = GetAsPtr(box, TextBox);
            mFontWidget->setCurrentSettings(txtBox->getFontSize(),
                                            txtBox->getFontFamily(),
                                            txtBox->getFontStyle());
        }
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

void MainWindow::disable() {
    disableEventFilter();
    mGrayOutWidget = new QWidget(this);
    mGrayOutWidget->setFixedSize(size());
    mGrayOutWidget->setStyleSheet(
                "QWidget { background-color: rgb(0, 0, 0, 125) }");
    mGrayOutWidget->show();
    mGrayOutWidget->update();
}

void MainWindow::enable() {
    if(mGrayOutWidget == nullptr) return;
    enableEventFilter();
    delete mGrayOutWidget;
    mGrayOutWidget = nullptr;
    callUpdateSchedulers();
}

int MainWindow::getCurrentFrame() {
    return mCanvasWindow->getCurrentFrame();
}

int MainWindow::getFrameCount() {
    return mCanvasWindow->getMaxFrame();
}
#include "GUI/GradientWidgets/gradientwidget.h"
void MainWindow::setCurrentFrame(int frame) {
    mFillStrokeSettings->getGradientWidget()->updateAfterFrameChanged(frame);
    mCanvasWindow->updateAfterFrameChanged(frame);

    callUpdateSchedulers();
}

void MainWindow::newFile() {
    if(askForSaving()) {
        closeProject();
        createNewCanvas();
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e) {
    if(mEventFilterDisabled) {
        return QMainWindow::eventFilter(obj, e);
    }
    QWidget *focusWidget = QApplication::focusWidget();
    if(focusWidget != nullptr) {
        if(focusWidget->property("forceHandleEvent").isValid()) return false;
    }
    if(e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(e);
        if(keyEvent->key() == Qt::Key_Delete && focusWidget != nullptr) {
            mEventFilterDisabled = true;
            bool widHandled = QCoreApplication::sendEvent(focusWidget, keyEvent);
            mEventFilterDisabled = false;
            if(widHandled) return false;
        }
        return processKeyEvent(keyEvent);
    } else if(e->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(e);
        if(isShiftPressed() && keyEvent->key() == Qt::Key_D) {
            return processKeyEvent(keyEvent);
        }
        if(isCtrlPressed()) {
            if(keyEvent->key() == Qt::Key_C ||
               keyEvent->key() == Qt::Key_V ||
               keyEvent->key() == Qt::Key_X ||
               keyEvent->key() == Qt::Key_D //||
/*                keyEvent->key() == Qt::Key_Up ||
                keyEvent->key() == Qt::Key_Down*/) {
                return processKeyEvent(keyEvent);
            }
        } else if(
             keyEvent->key() == Qt::Key_A ||
             keyEvent->key() == Qt::Key_I/* ||
             keyEvent->key() == Qt::Key_Delete*/) {
              return processKeyEvent(keyEvent);
        } else if(keyEvent->key() == Qt::Key_Delete) {
            if(focusWidget != nullptr) {
                mEventFilterDisabled = true;
                bool widHandled = QCoreApplication::sendEvent(focusWidget, keyEvent);
                mEventFilterDisabled = false;
                if(widHandled) return QMainWindow::eventFilter(obj, e);
            }
            return processKeyEvent(keyEvent);
        }
    } else if(e->type() == QEvent::KeyRelease) {
        finishUndoRedoSet();
    } else if(e->type() == QEvent::MouseButtonRelease) {
        finishUndoRedoSet();
    } else if(obj == mCanvasWindow->getCanvasWidget()) {
        if(e->type() == QEvent::Drop) {
            mCanvasWindow->dropEvent(static_cast<QDropEvent*>(e));
        } else if(e->type() == QEvent::DragEnter) {
            mCanvasWindow->dragEnterEvent(static_cast<QDragEnterEvent*>(e));
        } else if(e->type() == QEvent::FocusIn) {
            mCanvasWindow->getCanvasWidget();
        }
    }
    return QMainWindow::eventFilter(obj, e);
}

void MainWindow::closeEvent(QCloseEvent *e) {
    if(!closeProject()) {
        e->ignore();
    }
}

void MainWindow::updateDisplayedFillStrokeSettings() {
    PaintSettings* fillSettings;
    StrokeSettings* strokeSettings;
    mCanvasWindow->getDisplayedFillStrokeSettingsFromLastSelected(
                fillSettings, strokeSettings);
    mFillStrokeSettings->setCurrentSettings(
                fillSettings, strokeSettings);
}

void MainWindow::updateDisplayedFillStrokeSettingsIfNeeded() {
    if(mDisplayedFillStrokeSettingsUpdateNeeded) {
        mDisplayedFillStrokeSettingsUpdateNeeded = false;
        updateDisplayedFillStrokeSettings();
    }
}

void MainWindow::scheduleDisplayedFillStrokeSettingsUpdate() {
    mDisplayedFillStrokeSettingsUpdateNeeded = true;
}

bool MainWindow::processKeyEvent(QKeyEvent *event) {
    if(isActiveWindow()) {
        bool returnBool = false;
        if(mBoxesListAnimationDockWidget->processKeyEvent(event)) {
            returnBool = true;
        } else {
            returnBool = KeyFocusTarget::KFT_handleKeyEvent(event);
        }
        callUpdateSchedulers();
        return returnBool;
    }
    return false;
}

bool MainWindow::isEnabled() {
    return mGrayOutWidget == nullptr;
}

void MainWindow::clearAll() {
    mUpdateSchedulers.clear();
    setFileChangedSinceSaving(false);
    mObjectSettingsWidget->setMainTarget(nullptr);

    mBoxesListAnimationDockWidget->clearAll();
    mCurrentCanvasComboBox->clear();
    mCanvasWindow->clearAll();
    mFillStrokeSettings->clearAll();
//    foreach(ClipboardContainer *cont, mClipboardContainers) {
//        delete cont;
//    }
//    mClipboardContainers.clear();
    replaceClipboard(nullptr);
    FileSourcesCache::clearAll();
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
        disable();
        QString openPath = QFileDialog::getOpenFileName(this,
            "Open File", mCurrentFilePath, "AniVect Files (*.av)");
        if(!openPath.isEmpty()) {
            clearAll();
            setCurrentPath(openPath);
            loadAVFile(mCurrentFilePath);
        }
        enable();
        setFileChangedSinceSaving(false);
    }
}

void MainWindow::saveFile() {
    if(mCurrentFilePath.isEmpty()) {
        saveFileAs();
        return;
    }
    saveToFile(mCurrentFilePath);
    setFileChangedSinceSaving(false);
}

void MainWindow::saveFileAs() {
    disableEventFilter();
    QString saveAs = QFileDialog::getSaveFileName(this, "Save File",
                               mCurrentFilePath,
                               "AniVect Files (*.av)");
    enableEventFilter();
    if(!saveAs.isEmpty()) {
        setCurrentPath(saveAs);
        saveToFile(mCurrentFilePath);
    }
    setFileChangedSinceSaving(false);
}

void MainWindow::saveBackup() {
    QString backupPath = "backup/backup_%1.av";
    int id = 1;
    QFile backupFile(backupPath.arg(id));
    while(backupFile.exists()) {
        id++;
        backupFile.setFileName(backupPath.arg(id) );
    }
    saveToFile(backupPath.arg(id));
}

bool MainWindow::closeProject() {
    if(askForSaving()) {
        setCurrentPath("");
        clearAll();
        setFileChangedSinceSaving(false);
        return true;
    }
    return false;
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

void MainWindow::revert() {
    clearAll();
    loadAVFile(mCurrentFilePath);
    setFileChangedSinceSaving(false);
}

void MainWindow::undo() {
    if(mCurrentUndoRedoStack == nullptr) return;
    getUndoRedoStack()->undo();
    mCanvasWindow->updateHoveredElements();
    callUpdateSchedulers();
}

void MainWindow::redo() {
    if(mCurrentUndoRedoStack == nullptr) return;
    getUndoRedoStack()->redo();
    mCanvasWindow->updateHoveredElements();
    callUpdateSchedulers();
}

void MainWindow::setCurrentFrameForAllWidgets(const int &frame) {
    mBoxesListAnimationDockWidget->setCurrentFrame(frame);
}

Gradient *MainWindow::getLoadedGradientById(const int &id) {
    Q_FOREACH(Gradient *gradient, mLoadedGradientsList) {
        if(gradient->getLoadId() == id) {
            return gradient;
        }
    }
    return nullptr;
}

void MainWindow::clearLoadedGradientsList() {
    mLoadedGradientsList.clear();
}

void MainWindow::addLoadedGradient(Gradient *gradient) {
    mLoadedGradientsList << gradient;
}
