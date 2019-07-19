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
#include "taskexecutor.h"
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
#include "Animators/gradient.h"
#include "canvaswindowwrapper.h"
#include "scenelayout.h"
#include "GUI/GradientWidgets/gradientwidget.h"
#include "GUI/newcanvasdialog.h"
#include "GPUEffects/shadereffectprogram.h"
extern "C" {
    #include <libavformat/avformat.h>
}

MainWindow *MainWindow::sMainWindowInstance;

void MainWindow::keyPressEvent(QKeyEvent *event) {
    processKeyEvent(event);
}

int FONT_HEIGHT;
int MIN_WIDGET_DIM;
int KEY_RECT_SIZE;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      mMemoryHandler(new MemoryHandler(this)),
      mVideoEncoder(SPtrCreate(VideoEncoder)()),
      mDocument(mAudioHandler), mActions(mDocument) {
    sMainWindowInstance = this;
    FONT_HEIGHT = QApplication::fontMetrics().height();
    MIN_WIDGET_DIM = FONT_HEIGHT*4/3;
    KEY_RECT_SIZE = MIN_WIDGET_DIM*3/5;
    av_register_all();
    mAudioHandler.initializeAudio();

    connect(&mDocument, &Document::evFilePathChanged,
            this, &MainWindow::updateTitle);
    connect(&mDocument, &Document::documentChanged,
            this, [this]() {
        setFileChangedSinceSaving(true);
    });
    connect(&mDocument, &Document::activeSceneSet,
            this, &MainWindow::updateSettingsForCurrentCanvas);

    connect(&mDocument, &Document::currentBoxChanged,
            this, &MainWindow::setCurrentBox);

    QFile customSS(EnveSettings::sSettingsDir() + "/stylesheet.qss");
    if(customSS.exists()) {
        if(customSS.open(QIODevice::ReadOnly | QIODevice::Text)) {
            setStyleSheet(customSS.readAll());
            customSS.close();
        }
    } else {
        QFile file(":/styles/stylesheet.qss");
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            setStyleSheet(file.readAll());
            file.close();
        }
    }
    //setMouseTracking(true);
    BoxSingleWidget::loadStaticPixmaps();

//    for(int i = 0; i < ClipboardContainerType::CCT_COUNT; i++) {
//        mClipboardContainers << nullptr;
//    }

    //int nThreads = QThread::idealThreadCount();

    mDocument.setPath("");

    mFillStrokeSettingsDock = new QDockWidget("Fill and Stroke", this);
    //const auto fillStrokeSettingsScroll = new ScrollArea(this);
    mFillStrokeSettings = new FillStrokeSettingsWidget(mDocument, this);
    //fillStrokeSettingsScroll->setWidget(mFillStrokeSettings);
    const auto fillStrokeDockLabel = new QLabel("Fill and Stroke", this);
    fillStrokeDockLabel->setObjectName("dockLabel");
    fillStrokeDockLabel->setAlignment(Qt::AlignCenter);
    mFillStrokeSettingsDock->setTitleBarWidget(fillStrokeDockLabel);
    mFillStrokeSettingsDock->setWidget(mFillStrokeSettings);
    mFillStrokeSettingsDock->setFeatures(QDockWidget::DockWidgetMovable |
                                         QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, mFillStrokeSettingsDock);
    mFillStrokeSettingsDock->setMinimumWidth(MIN_WIDGET_DIM*10);
    mFillStrokeSettingsDock->setMaximumWidth(MIN_WIDGET_DIM*20);

    mBottomDock = new QDockWidget("Timeline", this);

    mBottomDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    mBottomDock->setTitleBarWidget(new QWidget());
    addDockWidget(Qt::BottomDockWidgetArea, mBottomDock);

    mEffectsLoader = new EffectsLoader;
    mEffectsLoader->initialize();
    connect(mEffectsLoader, &EffectsLoader::programChanged, this,
    [this](ShaderEffectProgram * program) {
        for(const auto& scene : mDocument.fScenes)
            scene->updateIfUsesProgram(program);
        mDocument.actionFinished();
    });

    mLayoutHandler = new LayoutHandler(mDocument, mAudioHandler);
    mBoxesListAnimationDockWidget =
            new BoxesListAnimationDockWidget(mDocument, mLayoutHandler, this);
    mBottomDock->setWidget(mBoxesListAnimationDockWidget);

    mBrushSettingsDock = new QDockWidget("Brush Settings", this);
    mBrushSettingsDock->setFeatures(QDockWidget::DockWidgetMovable |
                                    QDockWidget::DockWidgetFloatable);
    mBrushSettingsDock->setMinimumWidth(MIN_WIDGET_DIM*10);
    mBrushSettingsDock->setMaximumWidth(MIN_WIDGET_DIM*20);
    const auto brushDockLabel = new QLabel("Brush Settings", this);
    brushDockLabel->setObjectName("dockLabel");
    brushDockLabel->setAlignment(Qt::AlignCenter);
    mBrushSettingsDock->setTitleBarWidget(brushDockLabel);

    const int ctxt = BrushSelectionWidget::sCreateNewContext();
    mBrushSelectionWidget = new BrushSelectionWidget(ctxt, this);
    connect(mBrushSelectionWidget, &BrushSelectionWidget::currentBrushChanged,
            &mDocument, &Document::setBrush);
//    connect(mBrushSettingsWidget,
//            SIGNAL(brushReplaced(const Brush*,const Brush*)),
//            mCanvasWindow,
//            SLOT(replaceBrush(const Brush*,const Brush*)));

    mBrushSettingsDock->setWidget(mBrushSelectionWidget);

    addDockWidget(Qt::LeftDockWidgetArea, mBrushSettingsDock);
    mBrushSettingsDock->hide();

    mLeftDock = new QDockWidget("Current Object", this);
    mLeftDock->setFeatures(QDockWidget::DockWidgetMovable |
                           QDockWidget::DockWidgetFloatable);
    mLeftDock->setMinimumWidth(MIN_WIDGET_DIM*10);
    mLeftDock->setMaximumWidth(MIN_WIDGET_DIM*20);
    const auto leftDockLabel = new QLabel("Current Object", this);
    leftDockLabel->setObjectName("dockLabel");
    leftDockLabel->setAlignment(Qt::AlignCenter);
    mLeftDock->setTitleBarWidget(leftDockLabel);

    mObjectSettingsScrollArea = new ScrollArea(this);
    mObjectSettingsWidget = new BoxScrollWidget(
                mDocument, mObjectSettingsScrollArea);
    mObjectSettingsScrollArea->setWidget(mObjectSettingsWidget);
    mObjectSettingsWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_BR_SELECTED);
    mObjectSettingsWidget->getVisiblePartWidget()->
            setCurrentTarget(nullptr, SWT_TARGET_CURRENT_GROUP);

    connect(mObjectSettingsScrollArea->verticalScrollBar(),
            &QScrollBar::valueChanged,
            mObjectSettingsWidget, &BoxScrollWidget::changeVisibleTop);
    connect(mObjectSettingsScrollArea, &ScrollArea::heightChanged,
            mObjectSettingsWidget, &BoxScrollWidget::changeVisibleHeight);
    connect(mObjectSettingsScrollArea, &ScrollArea::widthChanged,
            mObjectSettingsWidget, &BoxScrollWidget::setWidth);

    mObjectSettingsScrollArea->verticalScrollBar()->setSingleStep(
                MIN_WIDGET_DIM);

    mLeftDock->setWidget(mObjectSettingsScrollArea);
    addDockWidget(Qt::LeftDockWidgetArea, mLeftDock);

    mLeftDock2 = new QDockWidget(this);
    mLeftDock2->setFeatures(QDockWidget::DockWidgetMovable |
                            QDockWidget::DockWidgetFloatable);
    mLeftDock2->setMinimumWidth(MIN_WIDGET_DIM*10);
    mLeftDock2->setMaximumWidth(MIN_WIDGET_DIM*20);

    mLeftDock2->setWidget(new FileSourceList(this));
    addDockWidget(Qt::LeftDockWidgetArea, mLeftDock2);

    const auto leftDock2Label = new QLabel("Files", this);
    leftDock2Label->setObjectName("dockLabel");
    leftDock2Label->setAlignment(Qt::AlignCenter);
    mLeftDock2->setTitleBarWidget(leftDock2Label);


//    mCanvasWindow->SWT_getAbstractionForWidget(
//                mBoxListWidget->getVisiblePartWidget());

//    Canvas *canvas = new Canvas(mFillStrokeSettings, mCanvasWindow);
//    canvas->prp_setName("Canvas 0");
//    mCanvasWindow->addCanvasToListAndSetAsCurrent(canvas);
//    mCanvas = mCanvasWindow->getCurrentCanvas();
//    mCurrentCanvasComboBox->addItem(mCanvas->prp_getName());

    setCentralWidget(mLayoutHandler->sceneLayout());

    setupToolBar();
    setupStatusBar();
    setupMenuBar();

    connectToolBarActions();

    //setCentralWidget(mCanvasWindow->getCanvasWidget());

    showMaximized();

    this->setMouseTracking(true);
    centralWidget()->setMouseTracking(true);

    readRecentFiles();
    updateRecentMenu();

    mEventFilterDisabled = false;

    try {
        mDocument.fTaskScheduler.initializeGPU();
    } catch(const std::exception& e) {
        gPrintExceptionFatal(e);
    }


    QApplication::instance()->installEventFilter(this);
}

MainWindow::~MainWindow() {
//    mtaskExecutorThread->terminate();
//    mtaskExecutorThread->quit();
    BoxSingleWidget::clearStaticPixmaps();
}

#include "noshortcutaction.h"
void MainWindow::setupMenuBar() {
    mMenuBar = new QMenuBar(this);

    mFileMenu = mMenuBar->addMenu("File");
    mFileMenu->addAction("New...",
                         this, &MainWindow::newFile,
                         Qt::CTRL + Qt::Key_N);
    mFileMenu->addAction("Open...",
                         this, qOverload<>(&MainWindow::openFile),
                         Qt::CTRL + Qt::Key_O);
    mRecentMenu = mFileMenu->addMenu("Open Recent");
    mFileMenu->addSeparator();
    mFileMenu->addAction("Link...",
                         this, &MainWindow::linkFile,
                         Qt::CTRL + Qt::Key_L);
    mFileMenu->addAction("Import File...",
                         this,
                         qOverload<>(&MainWindow::importFile),
                         Qt::CTRL + Qt::Key_I);
    mFileMenu->addAction("Import Image Sequence...",
                         this, &MainWindow::importImageSequence);
    mFileMenu->addSeparator();
    mFileMenu->addAction("Revert", this, &MainWindow::revert);
    mFileMenu->addSeparator();
    mFileMenu->addAction("Save",
                         this, &MainWindow::saveFile,
                         Qt::CTRL + Qt::Key_S);
    mFileMenu->addAction("Save As...",
                         this, &MainWindow::saveFileAs,
                         Qt::CTRL + Qt::SHIFT + Qt::Key_S);
    mFileMenu->addAction("Save Backup",
                         this, &MainWindow::saveBackup);
    mFileMenu->addSeparator();
    mFileMenu->addAction("Close", this, &MainWindow::closeProject);
    mFileMenu->addSeparator();
    mFileMenu->addAction("Exit", this, &MainWindow::close);

    mEditMenu = mMenuBar->addMenu("Edit");

    mEditMenu->addAction("Undo",
                         &mActions, &Actions::undoAction,
                         Qt::CTRL + Qt::Key_Z);
    mEditMenu->addAction("Redo",
                         &mActions, &Actions::redoAction,
                         Qt::CTRL + Qt::SHIFT + Qt::Key_Z);
    mEditMenu->addAction("Undo History...");
    mEditMenu->addSeparator();
    mEditMenu->addAction(new NoShortcutAction("Cut",
                         &mActions, &Actions::cutAction,
                         Qt::CTRL + Qt::Key_X, mEditMenu));
    mEditMenu->addAction(new NoShortcutAction("Copy",
                         &mActions, &Actions::copyAction,
                         Qt::CTRL + Qt::Key_C, mEditMenu));
    mEditMenu->addAction(new NoShortcutAction("Paste",
                         &mActions, &Actions::pasteAction,
                         Qt::CTRL + Qt::Key_V, mEditMenu));
    mEditMenu->addSeparator();
    mEditMenu->addAction(new NoShortcutAction("Duplicate",
                         &mActions, &Actions::duplicateAction,
                         Qt::CTRL + Qt::Key_D, mEditMenu));
    mEditMenu->addSeparator();
    mEditMenu->addAction(new NoShortcutAction("Delete",
                         &mActions, &Actions::deleteAction,
                         Qt::Key_Delete, mEditMenu));
    mEditMenu->addSeparator();
    mEditMenu->addAction(new NoShortcutAction("Select All",
                         &mActions, &Actions::selectAllAction,
                         Qt::Key_A, mEditMenu));
    mEditMenu->addAction(new NoShortcutAction("Invert Selection",
                                              &mActions, &Actions::invertSelectionAction,
                                              Qt::Key_I, mEditMenu));
    mEditMenu->addAction(new NoShortcutAction("Clear Selection",
                         &mActions, &Actions::clearSelectionAction,
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
                           &mActions, &Actions::raiseAction,
                           Qt::Key_PageUp);
    mObjectMenu->addAction("Lower",
                           &mActions, &Actions::lowerAction,
                           Qt::Key_PageDown);
    mObjectMenu->addAction("Rasie to Top",
                           &mActions, &Actions::raiseToTopAction,
                           Qt::Key_Home);
    mObjectMenu->addAction("Lower to Bottom",
                           &mActions, &Actions::lowerToBottomAction)->
            setShortcut(Qt::Key_End);
    mObjectMenu->addSeparator();
    mObjectMenu->addAction("Rotate 90° CW",
                           &mActions, &Actions::rotate90CWAction);
    mObjectMenu->addAction("Rotate 90° CCW",
                           &mActions, &Actions::rotate90CCWAction);
    mObjectMenu->addAction("Flip Horizontal", &mActions,
                           &Actions::flipHorizontalAction, Qt::Key_H);
    mObjectMenu->addAction("Flip Vertical", &mActions,
                           &Actions::flipVerticalAction, Qt::Key_V);
    mObjectMenu->addSeparator();
    mObjectMenu->addAction("Group", &mActions,
                           &Actions::groupSelectedBoxes,
                           Qt::CTRL + Qt::Key_G);
    mObjectMenu->addAction("Ungroup", &mActions,
                           &Actions::ungroupSelectedBoxes,
                           Qt::CTRL + Qt::SHIFT + Qt::Key_G);

    mPathMenu = mMenuBar->addMenu("Path");

    mPathMenu->addAction("Object to Path", &mActions,
                         &Actions::objectsToPathAction);
    mPathMenu->addAction("Stroke to Path", &mActions,
                         &Actions::strokeToPathAction);
    mPathMenu->addSeparator();
    mPathMenu->addAction("Union", &mActions,
                         &Actions::pathsUnionAction)->
            setShortcut(Qt::CTRL + Qt::Key_Plus);
    mPathMenu->addAction("Difference", &mActions,
                         &Actions::pathsDifferenceAction)->
            setShortcut(Qt::CTRL + Qt::Key_Minus);
    mPathMenu->addAction("Intersection", &mActions,
                         &Actions::pathsIntersectionAction)->
            setShortcut(Qt::CTRL + Qt::Key_Asterisk);
    mPathMenu->addAction("Exclusion", &mActions,
                         &Actions::pathsExclusionAction)->
            setShortcut(Qt::CTRL + Qt::Key_AsciiCircum);
    mPathMenu->addAction("Division", &mActions,
                         &Actions::pathsDivisionAction)->
            setShortcut(Qt::CTRL + Qt::Key_Slash);
//    mPathMenu->addAction("Cut Path", mCanvas,
//                         &Actions::pathsCutAction);
    mPathMenu->addSeparator();
    mPathMenu->addAction("Combine", &mActions,
                         &Actions::pathsCombineAction)->
            setShortcut(Qt::CTRL + Qt::Key_K);
    mPathMenu->addAction("Break Apart", &mActions,
                         &Actions::pathsBreakApartAction)->
            setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_K);

//    mEffectsMenu = mMenuBar->addMenu("Effects");

//    mEffectsMenu->addAction("Blur");

    mSceneMenu = mMenuBar->addMenu("Scene");
    mSceneMenu->addAction("New scene...", this, [this]() {
        CanvasSettingsDialog::sNewCanvasDialog(mDocument, this);
    });

    mViewMenu = mMenuBar->addMenu("View");

    const auto filteringMenu = mViewMenu->addMenu("Filtering");

    mNoneQuality = filteringMenu->addAction("None", [this]() {
        BoundingBox::sDisplayFiltering = kNone_SkFilterQuality;
        centralWidget()->update();

        mLowQuality->setChecked(false);
        mMediumQuality->setChecked(false);
        mHighQuality->setChecked(false);
    });
    mNoneQuality->setCheckable(true);
    mNoneQuality->setChecked(BoundingBox::sDisplayFiltering == kNone_SkFilterQuality);

    mLowQuality = filteringMenu->addAction("Low", [this]() {
        BoundingBox::sDisplayFiltering = kLow_SkFilterQuality;
        centralWidget()->update();

        mNoneQuality->setChecked(false);
        mMediumQuality->setChecked(false);
        mHighQuality->setChecked(false);
    });
    mLowQuality->setCheckable(true);
    mLowQuality->setChecked(BoundingBox::sDisplayFiltering == kLow_SkFilterQuality);

    mMediumQuality = filteringMenu->addAction("Medium", [this]() {
        BoundingBox::sDisplayFiltering = kMedium_SkFilterQuality;
        centralWidget()->update();

        mNoneQuality->setChecked(false);
        mLowQuality->setChecked(false);
        mHighQuality->setChecked(false);
    });
    mMediumQuality->setCheckable(true);
    mMediumQuality->setChecked(BoundingBox::sDisplayFiltering == kMedium_SkFilterQuality);

    mHighQuality = filteringMenu->addAction("High", [this]() {
        BoundingBox::sDisplayFiltering = kHigh_SkFilterQuality;
        centralWidget()->update();

        mNoneQuality->setChecked(false);
        mLowQuality->setChecked(false);
        mMediumQuality->setChecked(false);
    });
    mHighQuality->setCheckable(true);
    mHighQuality->setChecked(BoundingBox::sDisplayFiltering == kHigh_SkFilterQuality);

    mClipViewToCanvas = mViewMenu->addAction("Clip To Canvas");
    mClipViewToCanvas->setCheckable(true);
    mClipViewToCanvas->setChecked(true);
    mClipViewToCanvas->setShortcut(QKeySequence(Qt::Key_C));
    connect(mClipViewToCanvas, &QAction::toggled,
            &mActions, &Actions::setClipToCanvas);

    mRasterEffectsVisible = mViewMenu->addAction("Raster Effects");
    mRasterEffectsVisible->setCheckable(true);
    mRasterEffectsVisible->setChecked(true);
    connect(mRasterEffectsVisible, &QAction::toggled,
            &mActions, &Actions::setRasterEffectsVisible);

    mPathEffectsVisible = mViewMenu->addAction("Path Effects");
    mPathEffectsVisible->setCheckable(true);
    mPathEffectsVisible->setChecked(true);
    connect(mPathEffectsVisible, &QAction::toggled,
            &mActions, &Actions::setPathEffectsVisible);


    mPanelsMenu = mViewMenu->addMenu("Docks");

    mCurrentObjectDock = mPanelsMenu->addAction("Current Object");
    mCurrentObjectDock->setCheckable(true);
    mCurrentObjectDock->setChecked(true);
    mCurrentObjectDock->setShortcut(QKeySequence(Qt::Key_O));

    connect(mCurrentObjectDock, &QAction::toggled,
            mLeftDock, &QDockWidget::setVisible);

    mFilesDock = mPanelsMenu->addAction("Files");
    mFilesDock->setCheckable(true);
    mFilesDock->setChecked(true);
    mFilesDock->setShortcut(QKeySequence(Qt::Key_F));

    connect(mFilesDock, &QAction::toggled,
            mLeftDock2, &QDockWidget::setVisible);

    mObjectsAndAnimationsDock = mPanelsMenu->addAction("Objects and Animations");
    mObjectsAndAnimationsDock->setCheckable(true);
    mObjectsAndAnimationsDock->setChecked(true);
    mObjectsAndAnimationsDock->setShortcut(QKeySequence(Qt::Key_T));

    connect(mObjectsAndAnimationsDock, &QAction::toggled,
            mBottomDock, &QDockWidget::setVisible);

    mFillAndStrokeSettingsDock = mPanelsMenu->addAction("Fill and Stroke");
    mFillAndStrokeSettingsDock->setCheckable(true);
    mFillAndStrokeSettingsDock->setChecked(true);
    mFillAndStrokeSettingsDock->setShortcut(QKeySequence(Qt::Key_E));

    connect(mFillAndStrokeSettingsDock, &QAction::toggled,
            mFillStrokeSettingsDock, &QDockWidget::setVisible);

    mBrushSettingsDockAction = mPanelsMenu->addAction("Brush Settings");
    mBrushSettingsDockAction->setCheckable(true);
    mBrushSettingsDockAction->setChecked(false);
    mBrushSettingsDockAction->setShortcut(QKeySequence(Qt::Key_B));

    connect(mBrushSettingsDockAction, &QAction::toggled,
            mBrushSettingsDock, &QDockWidget::setVisible);

    mRenderMenu = mMenuBar->addMenu("Render");
    mRenderMenu->addAction("Render", this, &MainWindow::addCanvasToRenderQue);

    setMenuBar(mMenuBar);
    mMenuBar->setStyleSheet("QMenuBar { padding-top: 1px; }");
//

}

void MainWindow::addCanvasToRenderQue() {
    if(!mDocument.fActiveScene) return;
    mBoxesListAnimationDockWidget->getRenderWidget()->
    createNewRenderInstanceWidgetForCanvas(mDocument.fActiveScene);
}

void MainWindow::updateSettingsForCurrentCanvas(Canvas* const scene) {
    if(!scene) {
        mObjectSettingsWidget->setMainTarget(nullptr);
        //mBrushSettingsWidget->setCurrentBrush(nullptr);
        mBoxesListAnimationDockWidget->updateSettingsForCurrentCanvas(nullptr);
        return;
    }
    mClipViewToCanvas->setChecked(scene->clipToCanvas());
    mRasterEffectsVisible->setChecked(scene->getRasterEffectsVisible());
    mPathEffectsVisible->setChecked(scene->getPathEffectsVisible());
    mBoxesListAnimationDockWidget->updateSettingsForCurrentCanvas(scene);
    mObjectSettingsWidget->setMainTarget(scene->getCurrentGroup());
//    mBrushSettingsWidget->setCurrentBrush(canvas->getCurrentBrush());
}

#include <QSpacerItem>
void MainWindow::setupStatusBar() {
    mUsageWidget = new UsageWidget(this);
    mUsageWidget->setStyleSheet("QStatusBar { border-top: 1px solid black; }");
    setStatusBar(mUsageWidget);
}

void MainWindow::setupToolBar() {
    mToolBar = new QToolBar("Toolbar", this);
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
                "F4", this);
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

    mPaintMode = new ActionButton(
                ":/icons/paint_mode.png",
                "F11", this);
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

    mToolBar->addWidget(mLayoutHandler->comboWidget());

    addToolBar(mToolBar);

    mDocument.setCanvasMode(CanvasMode::MOVE_BOX);
}

void MainWindow::connectToolBarActions() {
    connect(mMovePathMode, &ActionButton::pressed,
            &mActions, &Actions::setMovePathMode);
    connect(mMovePointMode, &ActionButton::pressed,
            &mActions, &Actions::setMovePointMode);
    connect(mAddPointMode, &ActionButton::pressed,
            &mActions, &Actions::setAddPointMode);
    connect(mPickPaintSettingsMode, &ActionButton::pressed,
            &mActions, &Actions::setPickPaintSettingsMode);
    connect(mCircleMode, &ActionButton::pressed,
            &mActions, &Actions::setCircleMode);
    connect(mRectangleMode, &ActionButton::pressed,
            &mActions, &Actions::setRectangleMode);
    connect(mTextMode, &ActionButton::pressed,
            &mActions, &Actions::setTextMode);
    connect(mParticleBoxMode, &ActionButton::pressed,
            &mActions, &Actions::setParticleBoxMode);
    connect(mParticleEmitterMode, &ActionButton::pressed,
            &mActions, &Actions::setParticleEmitterMode);
    connect(mPaintMode, &ActionButton::pressed,
            &mActions, &Actions::setPaintMode);
    connect(mActionConnectPoints, &ActionButton::pressed,
            &mActions, &Actions::connectPointsSlot);
    connect(mActionDisconnectPoints, &ActionButton::pressed,
            &mActions, &Actions::disconnectPointsSlot);
    connect(mActionMergePoints, &ActionButton::pressed,
            &mActions, &Actions::mergePointsSlot);
    connect(mActionSymmetricPointCtrls, &ActionButton::pressed,
            &mActions, &Actions::makePointCtrlsSymmetric);
    connect(mActionSmoothPointCtrls, &ActionButton::pressed,
            &mActions, &Actions::makePointCtrlsSmooth);
    connect(mActionCornerPointCtrls, &ActionButton::pressed,
            &mActions, &Actions::makePointCtrlsCorner);
    connect(mActionLine, &ActionButton::pressed,
            &mActions, &Actions::makeSegmentLine);
    connect(mActionCurve, &ActionButton::pressed,
            &mActions, &Actions::makeSegmentCurve);
    connect(mFontWidget, &FontsWidget::fontSizeChanged,
            &mActions, &Actions::setFontSize);
    connect(mFontWidget, &FontsWidget::fontFamilyAndStyleChanged,
            &mActions, &Actions::setFontFamilyAndStyle);
}

MainWindow *MainWindow::getInstance() {
    return sMainWindowInstance;
}

void MainWindow::updateCanvasModeButtonsChecked() {
    const CanvasMode currentMode = mDocument.fCanvasMode;
    mMovePathMode->setChecked(currentMode == MOVE_BOX);
    mMovePointMode->setChecked(currentMode == MOVE_POINT);
    mAddPointMode->setChecked(currentMode == ADD_POINT);
    mPickPaintSettingsMode->setChecked(currentMode == PICK_PAINT_SETTINGS);
    mCircleMode->setChecked(currentMode == ADD_CIRCLE);
    mRectangleMode->setChecked(currentMode == ADD_RECTANGLE);
    mTextMode->setChecked(currentMode == ADD_TEXT);
    mParticleBoxMode->setChecked(currentMode == ADD_PARTICLE_BOX);
    mParticleEmitterMode->setChecked(currentMode == ADD_PARTICLE_EMITTER);
    mPaintMode->setChecked(currentMode == PAINT_MODE);
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

void MainWindow::setResolutionFractionValue(const qreal value) {
    mDocument.fActiveScene->setResolutionFraction(value);
}

void MainWindow::setFileChangedSinceSaving(const bool changed) {
    if(changed == mChangedSinceSaving) return;
    mChangedSinceSaving = changed;
    updateTitle();
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

SimpleBrushWrapper *MainWindow::getCurrentBrush() const {
    return mBrushSelectionWidget->getCurrentBrush();
}

#include "Boxes/textbox.h"
void MainWindow::setCurrentBox(BoundingBox *box) {
    if(!box) {
        mFillStrokeSettings->setCurrentSettings(nullptr, nullptr);
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
                                      mDocument.fEvFile.split("/").last() +
                                      "\"?", "Close without saving",
                                      "Cancel",
                                      "Save");
        if(buttonId == 1) {
            return false;
        } else if(buttonId == 2) {
            saveFile();
            return true;
        }
    }
    return true;
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
    if(!mGrayOutWidget) return;
    enableEventFilter();
    delete mGrayOutWidget;
    mGrayOutWidget = nullptr;
    mDocument.actionFinished();
}

void MainWindow::newFile() {
    if(askForSaving()) {
        closeProject();
        CanvasSettingsDialog::sNewCanvasDialog(mDocument, this);
    }
}

bool handleCanvasModeKeyPress(Document& document, const int key) {
    if(key == Qt::Key_F1) {
        document.setCanvasMode(CanvasMode::MOVE_BOX);
    } else if(key == Qt::Key_F2) {
        document.setCanvasMode(CanvasMode::MOVE_POINT);
    } else if(key == Qt::Key_F3) {
            //setCanvasMode(CanvasMode::ADD_POINT);
        document.setCanvasMode(CanvasMode::ADD_POINT);
    } else if(key == Qt::Key_F4) {
        document.setCanvasMode(CanvasMode::PICK_PAINT_SETTINGS);
    } else if(key == Qt::Key_F6) {
        document.setCanvasMode(CanvasMode::ADD_CIRCLE);
    } else if(key == Qt::Key_F7) {
        document.setCanvasMode(CanvasMode::ADD_RECTANGLE);
    } else if(key == Qt::Key_F8) {
        document.setCanvasMode(CanvasMode::ADD_TEXT);
    } else if(key == Qt::Key_F9) {
        document.setCanvasMode(CanvasMode::ADD_PARTICLE_BOX);
    } else if(key == Qt::Key_F10) {
        document.setCanvasMode(CanvasMode::ADD_PARTICLE_EMITTER);
    } else if(key == Qt::Key_F11) {
        document.setCanvasMode(CanvasMode::PAINT_MODE);
    } else return false;
    return true;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e) {
    if(mLock) if(dynamic_cast<QInputEvent*>(e)) return true;
    if(mEventFilterDisabled) {
        return QMainWindow::eventFilter(obj, e);
    }
    const auto type = e->type();
    const auto focusWidget = QApplication::focusWidget();
    if(focusWidget) {
        if(focusWidget->property("forceHandleEvent").isValid()) return false;
    }
    if(type == QEvent::KeyPress) {
        const auto keyEvent = static_cast<QKeyEvent*>(e);
        if(keyEvent->key() == Qt::Key_Delete && focusWidget) {
            mEventFilterDisabled = true;
            const bool widHandled =
                    QCoreApplication::sendEvent(focusWidget, keyEvent);
            mEventFilterDisabled = false;
            if(widHandled) return false;
        }
        return processKeyEvent(keyEvent);
    } else if(type == QEvent::ShortcutOverride) {
        const auto keyEvent = static_cast<QKeyEvent*>(e);
        const int key = keyEvent->key();
        if(handleCanvasModeKeyPress(mDocument, key)) return true;
        if(isShiftPressed() && key == Qt::Key_D) {
            return processKeyEvent(keyEvent);
        }
        if(isCtrlPressed()) {
            if(key == Qt::Key_C || key == Qt::Key_V ||
               key == Qt::Key_X || key == Qt::Key_D //||
/*                key == Qt::Key_Up ||
                key == Qt::Key_Down*/) {
                return processKeyEvent(keyEvent);
            }
        } else if(key == Qt::Key_A || key == Qt::Key_I ||
                  key == Qt::Key_Delete) {
              return processKeyEvent(keyEvent);
        }
    } else if(type == QEvent::KeyRelease) {
        //finishUndoRedoSet();
    } else if(type == QEvent::MouseButtonRelease) {
        //finishUndoRedoSet();
    }
    return QMainWindow::eventFilter(obj, e);
}

void MainWindow::closeEvent(QCloseEvent *e) {
    if(!closeProject()) {
        e->ignore();
    }
}

bool MainWindow::processKeyEvent(QKeyEvent *event) {
    if(isActiveWindow()) {
        bool returnBool = false;
        if(mBoxesListAnimationDockWidget->processKeyEvent(event)) {
            returnBool = true;
        } else {
            returnBool = KeyFocusTarget::KFT_handleKeyEvent(event);
        }
        if(event->key() != Qt::Key_Control) mDocument.actionFinished();
        return returnBool;
    }
    return false;
}

bool MainWindow::isEnabled() {
    return !mGrayOutWidget;
}

void MainWindow::clearAll() {
    mTaskScheduler.clearTasks();
    setFileChangedSinceSaving(false);
    mObjectSettingsWidget->setMainTarget(nullptr);

    mBoxesListAnimationDockWidget->clearAll();
    mFillStrokeSettings->clearAll();
    mDocument.clear();
    mLayoutHandler->clear();
//    for(ClipboardContainer *cont : mClipboardContainers) {
//        delete cont;
//    }
//    mClipboardContainers.clear();
    FileCacheHandler::sClear();
    //mBoxListWidget->clearAll();
}

void MainWindow::updateTitle() {
    QString star = "";
    if(mChangedSinceSaving) star = "*";
    setWindowTitle(mDocument.fEvFile.split("/").last() + star + " - enve");
}

void MainWindow::openFile() {
    if(askForSaving()) {
        disable();
        const QString openPath = QFileDialog::getOpenFileName(this,
            "Open File", mDocument.fEvFile, "enve Files (*.ev)");
        if(!openPath.isEmpty()) openFile(openPath);
        enable();
    }
}

void MainWindow::openFile(const QString& openPath) {
    clearAll();
    try {
        loadEVFile(openPath);
        mDocument.setPath(openPath);
        setFileChangedSinceSaving(false);
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
    }
}

void MainWindow::saveFile() {
    if(mDocument.fEvFile.isEmpty()) {
        saveFileAs();
        return;
    }
    try {
        saveToFile(mDocument.fEvFile);
        setFileChangedSinceSaving(false);
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
    }
}

void MainWindow::saveFileAs() {
    disableEventFilter();
    const QString saveAs = QFileDialog::getSaveFileName(this, "Save File",
                               mDocument.fEvFile,
                               "enve Files (*.ev)");
    enableEventFilter();
    if(!saveAs.isEmpty()) {
        try {
            saveToFile(saveAs);
            mDocument.setPath(saveAs);
            setFileChangedSinceSaving(false);
        } catch(const std::exception& e) {
            gPrintExceptionCritical(e);
        }
    }
}

void MainWindow::saveBackup() {
    const QString backupPath = "backup/backup_%1.ev";
    int id = 1;
    QFile backupFile(backupPath.arg(id));
    while(backupFile.exists()) {
        id++;
        backupFile.setFileName(backupPath.arg(id) );
    }
    try {
        saveToFile(backupPath.arg(id));
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
    }
}

bool MainWindow::closeProject() {
    if(askForSaving()) {
        mDocument.setPath("");
        clearAll();
        return true;
    }
    return false;
}

void MainWindow::importFile() {
    disableEventFilter();
    QStringList importPaths = QFileDialog::getOpenFileNames(
                                            this,
                                            "Import File", "",
                                            "Files (*.ev *.svg "
                                                   "*.mp4 *.mov *.avi *.mkv *.m4v "
                                                   "*.png *.jpg "
                                                   "*.wav *.mp3)");
    enableEventFilter();
    if(!importPaths.isEmpty()) {
        for(const QString &path : importPaths) {
            if(path.isEmpty()) continue;
            try {
                mActions.importFile(path);
            } catch(const std::exception& e) {
                gPrintExceptionCritical(e);
            }
        }
    }
}

void MainWindow::linkFile() {
    disableEventFilter();
    QStringList importPaths = QFileDialog::getOpenFileNames(this,
        "Link File", "", "enve Files (*.ev)");
    enableEventFilter();
    if(!importPaths.isEmpty()) {
        for(const QString &path : importPaths) {
            if(path.isEmpty()) continue;
            mDocument.fActiveScene->createLinkToFileWithPath(path);
        }
    }
}

void MainWindow::importImageSequence() {
    disableEventFilter();
    const auto folder = QFileDialog::getExistingDirectory(
                this, "Import Image Sequence", "");
    enableEventFilter();
    if(!folder.isEmpty()) {
        mDocument.fActiveScene->createAnimationBoxForPaths(folder);
    }
    mDocument.actionFinished();
}

//void MainWindow::importVideo() {
//    disableEventFilter();
//    QStringList importPaths = QFileDialog::getOpenFileNames(this,
//        "Import Video", "", "Video (*.mp4 *.mov *.avi)");
//    enableEventFilter();
//    for(const QString &path : importPaths) {
//        mCanvasWindow->createVideoForPath(path);
//    }
//    callUpdateSchedulers();
//}

void MainWindow::revert() {
    clearAll();
    try {
        loadEVFile(mDocument.fEvFile);
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
    }
    setFileChangedSinceSaving(false);
}

stdsptr<void> MainWindow::lock() {
    if(mLock) return GetAsSPtr(mLock, Lock);
    setEnabled(false);
    const auto newLock = SPtrCreate(Lock)(this);
    mLock = newLock.get();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    return newLock;
}

void MainWindow::lockFinished() {
    if(mLock) {
        gPrintException(false, "Lock finished before lock object deleted");
    } else {
        QApplication::restoreOverrideCursor();
        setEnabled(true);
    }
}

void MainWindow::updateRecentMenu() {
    mRecentMenu->clear();
    const auto homePath = QDir::homePath();
    for(const auto& path : mRecentFiles) {
        QString ttPath = path;
        if(ttPath.left(homePath.count()) == homePath) {
            ttPath = "~" + ttPath.mid(homePath.count());
        }
        mRecentMenu->addAction(path, [path, this]() {
            openFile(path);
        });
    }
}
