// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "mainwindow.h"
#include "canvas.h"
#include <QKeyEvent>
#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include <QStatusBar>
#include "usagewidget.h"
#include <QToolBar>
#include "GUI/ColorWidgets/colorsettingswidget.h"
#include <QMenuBar>
#include "GUI/edialogs.h"
#include <QMessageBox>
#include "timelinedockwidget.h"
#include "Private/Tasks/taskexecutor.h"
#include "qdoubleslider.h"
#include "svgimporter.h"
#include "canvaswindow.h"
#include "GUI/BoxesList/boxscrollwidget.h"
#include "clipboardcontainer.h"
#include "GUI/BoxesList/OptimalScrollArea/scrollarea.h"
#include "GUI/BoxesList/boxscroller.h"
#include "GUI/RenderWidgets/renderwidget.h"
#include "actionbutton.h"
#include "fontswidget.h"
#include "GUI/global.h"
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
#include "GUI/GradientWidgets/gradientwidget.h"
#include "GUI/newcanvasdialog.h"
#include "ShaderEffects/shadereffectprogram.h"
#include "importhandler.h"
#include "envesplash.h"
#include "envelicense.h"
#include "switchbutton.h"
#include "centralwidget.h"
#include "ColorWidgets/bookmarkedcolors.h"
#include "GUI/edialogs.h"
#include "closesignalingdockwidget.h"

MainWindow *MainWindow::sInstance = nullptr;

void MainWindow::keyPressEvent(QKeyEvent *event) {
    processKeyEvent(event);
}

class evImporter : public eImporter {
public:
    bool supports(const QFileInfo& fileInfo) const {
        return fileInfo.suffix() == "ev";
    }

    qsptr<BoundingBox> import(const QFileInfo& fileInfo,
                              Canvas* const scene) const {
        Q_UNUSED(scene);
        MainWindow::sGetInstance()->loadEVFile(fileInfo.absoluteFilePath());
        return nullptr;
    }
};

class eSvgImporter : public eImporter {
public:
    bool supports(const QFileInfo& fileInfo) const {
        return fileInfo.suffix() == "svg";
    }

    qsptr<BoundingBox> import(const QFileInfo& fileInfo,
                              Canvas* const scene) const {
        return loadSVGFile(fileInfo.absoluteFilePath(), scene);
    }
};

MainWindow::MainWindow(Document& document,
                       Actions& actions,
                       AudioHandler& audioHandler,
                       RenderHandler& renderHandler,
                       QWidget * const parent)
    : QMainWindow(parent),
      mDocument(document),
      mActions(actions),
      mAudioHandler(audioHandler),
      mRenderHandler(renderHandler) {
    Q_ASSERT(!sInstance);
    sInstance = this;

    ImportHandler::sInstance->addImporter<evImporter>();
    ImportHandler::sInstance->addImporter<eSvgImporter>();

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
    connect(&mDocument, &Document::canvasModeSet,
            this, &MainWindow::updateCanvasModeButtonsChecked);
    connect(&mDocument, &Document::sceneCreated,
            this, &MainWindow::closeWelcomeDialog);

    const auto iconDir = eSettings::sIconsDir();
    setWindowIcon(QIcon(iconDir + "/enve.png"));
    const auto downArr = iconDir + "/down-arrow.png";
    const auto upArr = iconDir + "/up-arrow.png";
    const QString iconSS =
            "QComboBox::down-arrow { image: url(" + downArr + "); }" +
            "QScrollBar::sub-line { image: url(" + upArr + "); }" +
            "QScrollBar::add-line { image: url(" + downArr + "); }";

    QFile customSS(eSettings::sSettingsDir() + "/stylesheet.qss");
    if(customSS.exists()) {
        if(customSS.open(QIODevice::ReadOnly | QIODevice::Text)) {
            setStyleSheet(customSS.readAll());
            customSS.close();
        }
    } else {
        QFile file(":/styles/stylesheet.qss");
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            setStyleSheet(file.readAll() + iconSS);
            file.close();
        }
    }
    BoxSingleWidget::loadStaticPixmaps();

    BrushSelectionWidget::sPaintContext = BrushSelectionWidget::sCreateNewContext();
    BrushSelectionWidget::sOutlineContext = BrushSelectionWidget::sCreateNewContext();

//    for(int i = 0; i < ClipboardContainerType::CCT_COUNT; i++) {
//        mClipboardContainers << nullptr;
//    }

    mDocument.setPath("");

    mFillStrokeSettingsDock = new CloseSignalingDockWidget("Fill and Stroke", this);
    //const auto fillStrokeSettingsScroll = new ScrollArea(this);
    mFillStrokeSettings = new FillStrokeSettingsWidget(mDocument, this);
    //fillStrokeSettingsScroll->setWidget(mFillStrokeSettings);
    mFillStrokeSettingsDock->setWidget(mFillStrokeSettings);
    addDockWidget(Qt::RightDockWidgetArea, mFillStrokeSettingsDock);
    mFillStrokeSettingsDock->setMinimumWidth(MIN_WIDGET_DIM*12);
    mFillStrokeSettingsDock->setMaximumWidth(MIN_WIDGET_DIM*20);

    mTimelineDock = new CloseSignalingDockWidget("Timeline", this);
    mTimelineDock->setTitleBarWidget(new QWidget());
    addDockWidget(Qt::BottomDockWidgetArea, mTimelineDock);

    mLayoutHandler = new LayoutHandler(mDocument, mAudioHandler);
    mTimeline = new TimelineDockWidget(mDocument, mLayoutHandler, this);
    mTimelineDock->setWidget(mTimeline);

    mBrushSettingsDock = new CloseSignalingDockWidget("Brush Settings", this);
    mBrushSettingsDock->setMinimumWidth(MIN_WIDGET_DIM*10);
    mBrushSettingsDock->setMaximumWidth(MIN_WIDGET_DIM*20);

    const auto pCtxt = BrushSelectionWidget::sPaintContext;
    mBrushSelectionWidget = new BrushSelectionWidget(*pCtxt.get(), this);
    connect(mBrushSelectionWidget, &BrushSelectionWidget::currentBrushChanged,
            &mDocument, &Document::setBrush);
//    connect(mBrushSettingsWidget,
//            SIGNAL(brushReplaced(const Brush*,const Brush*)),
//            mCanvasWindow,
//            SLOT(replaceBrush(const Brush*,const Brush*)));

    mBrushSettingsDock->setWidget(mBrushSelectionWidget);

    addDockWidget(Qt::LeftDockWidgetArea, mBrushSettingsDock);
    mBrushSettingsDock->hide();

    mSelectedObjectDock = new CloseSignalingDockWidget("Selected Objects", this);
    mSelectedObjectDock->setMinimumWidth(MIN_WIDGET_DIM*10);
    mSelectedObjectDock->setMaximumWidth(MIN_WIDGET_DIM*20);

    mObjectSettingsScrollArea = new ScrollArea(this);
    mObjectSettingsWidget = new BoxScrollWidget(
                mDocument, mObjectSettingsScrollArea);
    mObjectSettingsScrollArea->setWidget(mObjectSettingsWidget);
    mObjectSettingsWidget->setCurrentRule(SWT_BoxRule::selected);
    mObjectSettingsWidget->setCurrentTarget(nullptr, SWT_Target::group);

    connect(mObjectSettingsScrollArea->verticalScrollBar(),
            &QScrollBar::valueChanged,
            mObjectSettingsWidget, &BoxScrollWidget::changeVisibleTop);
    connect(mObjectSettingsScrollArea, &ScrollArea::heightChanged,
            mObjectSettingsWidget, &BoxScrollWidget::changeVisibleHeight);
    connect(mObjectSettingsScrollArea, &ScrollArea::widthChanged,
            mObjectSettingsWidget, &BoxScrollWidget::setWidth);

    mObjectSettingsScrollArea->verticalScrollBar()->setSingleStep(
                MIN_WIDGET_DIM);

    mSelectedObjectDock->setWidget(mObjectSettingsScrollArea);
    addDockWidget(Qt::LeftDockWidgetArea, mSelectedObjectDock);

    mFilesDock = new CloseSignalingDockWidget("Files", this);
    mFilesDock->setMinimumWidth(MIN_WIDGET_DIM*10);
    mFilesDock->setMaximumWidth(MIN_WIDGET_DIM*20);

    mFilesDock->setWidget(new FileSourceList(this));
    addDockWidget(Qt::LeftDockWidgetArea, mFilesDock);


    {
        const auto brush = BrushCollectionData::sGetBrush("Deevad", "2B_pencil");
        const auto paintCtxt = BrushSelectionWidget::sPaintContext;
        const auto paintBrush = paintCtxt->brushWrapper(brush);
        mDocument.setBrush(paintBrush);
        mDocument.fOutlineBrush = brush;
    }
    const auto bBrush = new BookmarkedBrushes(true, 64, pCtxt.get(), this);
    const auto bColor = new BookmarkedColors(true, 64, this);

    mCentralWidget = new CentralWidget(bBrush,
                                       mLayoutHandler->sceneLayout(),
                                       bColor);

    setupToolBar();
    setupStatusBar();
    setupMenuBar();

    connectToolBarActions();

    readRecentFiles();
    updateRecentMenu();

    mEventFilterDisabled = false;

    installEventFilter(this);

    openWelcomeDialog();
    showMaximized();
}

MainWindow::~MainWindow() {
    sInstance = nullptr;
//    mtaskExecutorThread->terminate();
//    mtaskExecutorThread->quit();
    BoxSingleWidget::clearStaticPixmaps();
}

#include "noshortcutaction.h"
#include "efiltersettings.h"
#include "Settings/settingsdialog.h"
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
//    mFileMenu->addAction("Link...",
//                         this, &MainWindow::linkFile,
//                         Qt::CTRL + Qt::Key_L);
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

    const auto undoQAct = mEditMenu->addAction("Undo");
    undoQAct->setShortcut(Qt::CTRL + Qt::Key_Z);
    mActions.undoAction->connect(undoQAct);

    const auto redoQAct = mEditMenu->addAction("Redo");
    redoQAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Z);
    mActions.redoAction->connect(redoQAct);

    mEditMenu->addSeparator();

    {
        const auto qAct = new NoShortcutAction("Copy");
        mEditMenu->addAction(qAct);
        qAct->setShortcut(Qt::CTRL + Qt::Key_C);
        mActions.copyAction->connect(qAct);
    }

    {
        const auto qAct = new NoShortcutAction("Cut");
        mEditMenu->addAction(qAct);
        qAct->setShortcut(Qt::CTRL + Qt::Key_X);
        mActions.cutAction->connect(qAct);
    }

    {
        const auto qAct = new NoShortcutAction("Paste");
        mEditMenu->addAction(qAct);
        qAct->setShortcut(Qt::CTRL + Qt::Key_V);
        mActions.pasteAction->connect(qAct);
    }

    mEditMenu->addSeparator();

    {
        const auto qAct = new NoShortcutAction("Duplicate");
        mEditMenu->addAction(qAct);
        qAct->setShortcut(Qt::CTRL + Qt::Key_D);
        mActions.duplicateAction->connect(qAct);
    }

    mEditMenu->addSeparator();

    {
        const auto qAct = new NoShortcutAction("Delete");
        mEditMenu->addAction(qAct);
        qAct->setShortcut(Qt::Key_Delete);
        mActions.deleteAction->connect(qAct);
    }

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
    mEditMenu->addSeparator();
    mEditMenu->addAction("Settings...", [this]() {
        const auto settDial = new SettingsDialog(this);
        settDial->exec();
    });


//    mSelectSameMenu = mEditMenu->addMenu("Select Same");
//    mSelectSameMenu->addAction("Fill and Stroke");
//    mSelectSameMenu->addAction("Fill Color");
//    mSelectSameMenu->addAction("Stroke Color");
//    mSelectSameMenu->addAction("Stroke Style");
//    mSelectSameMenu->addAction("Object Type");

    mObjectMenu = mMenuBar->addMenu("Object");

    mObjectMenu->addSeparator();

    const auto raiseQAct = mObjectMenu->addAction("Raise");
    raiseQAct->setShortcut(Qt::Key_PageUp);
    mActions.raiseAction->connect(raiseQAct);

    const auto lowerQAct = mObjectMenu->addAction("Lower");
    lowerQAct->setShortcut(Qt::Key_PageDown);
    mActions.lowerAction->connect(lowerQAct);

    const auto rttQAct = mObjectMenu->addAction("Raise to Top");
    rttQAct->setShortcut(Qt::Key_Home);
    mActions.raiseToTopAction->connect(rttQAct);

    const auto ltbQAct = mObjectMenu->addAction("Lower to Bottom");
    ltbQAct->setShortcut(Qt::Key_End);
    mActions.lowerToBottomAction->connect(ltbQAct);

    mObjectMenu->addSeparator();

    {
        const auto qAct = mObjectMenu->addAction("Rotate 90° CW");
        mActions.rotate90CWAction->connect(qAct);
    }

    {
        const auto qAct = mObjectMenu->addAction("Rotate 90° CCW");
        mActions.rotate90CCWAction->connect(qAct);
    }

    {
        const auto qAct = mObjectMenu->addAction("Flip Horizontal");
        qAct->setShortcut(Qt::Key_H);
        mActions.flipHorizontalAction->connect(qAct);
    }

    {
        const auto qAct = mObjectMenu->addAction("Flip Vertical");
        qAct->setShortcut(Qt::Key_V);
        mActions.flipVerticalAction->connect(qAct);
    }

    mObjectMenu->addSeparator();

    const auto groupQAct = mObjectMenu->addAction("Group");
    groupQAct->setShortcut(Qt::CTRL + Qt::Key_G);
    mActions.groupAction->connect(groupQAct);

    const auto ungroupQAct = mObjectMenu->addAction("Ungroup");
    ungroupQAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_G);
    mActions.ungroupAction->connect(ungroupQAct);

    mObjectMenu->addSeparator();

    const auto transformMenu = mObjectMenu->addMenu("Transform");
    const auto moveAct = transformMenu->addAction("Move");
    moveAct->setShortcut(Qt::Key_G);
    moveAct->setDisabled(true);
    const auto rotateAct = transformMenu->addAction("Rotate");
    rotateAct->setShortcut(Qt::Key_R);
    rotateAct->setDisabled(true);
    const auto scaleAct = transformMenu->addAction("Scale");
    scaleAct->setShortcut(Qt::Key_S);
    scaleAct->setDisabled(true);
    transformMenu->addSeparator();
    const auto xAct = transformMenu->addAction("X-Axis Only");
    xAct->setShortcut(Qt::Key_X);
    xAct->setDisabled(true);
    const auto yAct = transformMenu->addAction("Y-Axis Only");
    yAct->setShortcut(Qt::Key_Y);
    yAct->setDisabled(true);


    mPathMenu = mMenuBar->addMenu("Path");

    const auto otpQAct = mPathMenu->addAction("Object to Path");
    mActions.objectsToPathAction->connect(otpQAct);

    const auto stpQAct = mPathMenu->addAction("Stroke to Path");
    mActions.strokeToPathAction->connect(stpQAct);

    mPathMenu->addSeparator();

    {
        const auto qAct = mPathMenu->addAction("Object to Sculpted Path");
        mActions.objectsToSculptedPathAction->connect(qAct);
    }

    mPathMenu->addSeparator();

    {
        const auto qAct = mPathMenu->addAction("Union");
        qAct->setShortcut(Qt::CTRL + Qt::Key_Plus);
        mActions.pathsUnionAction->connect(qAct);
    }

    {
        const auto qAct = mPathMenu->addAction("Difference");
        qAct->setShortcut(Qt::CTRL + Qt::Key_Minus);
        mActions.pathsDifferenceAction->connect(qAct);
    }

    {
        const auto qAct = mPathMenu->addAction("Intersection");
        qAct->setShortcut(Qt::CTRL + Qt::Key_Asterisk);
        mActions.pathsIntersectionAction->connect(qAct);
    }

    {
        const auto qAct = mPathMenu->addAction("Exclusion");
        qAct->setShortcut(Qt::CTRL + Qt::Key_AsciiCircum);
        mActions.pathsExclusionAction->connect(qAct);
    }

    {
        const auto qAct = mPathMenu->addAction("Division");
        qAct->setShortcut(Qt::CTRL + Qt::Key_Slash);
        mActions.pathsDivisionAction->connect(qAct);
    }


//    mPathMenu->addAction("Cut Path", mCanvas,
//                         &Actions::pathsCutAction);

    mPathMenu->addSeparator();

    {
        const auto qAct = mPathMenu->addAction("Combine");
        qAct->setShortcut(Qt::CTRL + Qt::Key_K);
        mActions.pathsCombineAction->connect(qAct);
    }

    {
        const auto qAct = mPathMenu->addAction("Break Apart");
        qAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_K);
        mActions.pathsBreakApartAction->connect(qAct);
    }

//    mEffectsMenu = mMenuBar->addMenu("Effects");

//    mEffectsMenu->addAction("Blur");

    mSceneMenu = mMenuBar->addMenu("Scene");
    mSceneMenu->addAction("New scene...", this, [this]() {
        CanvasSettingsDialog::sNewCanvasDialog(mDocument, this);
    });

    mSceneMenu->addAction("Add to Render Queue", this, &MainWindow::addCanvasToRenderQue);

    mViewMenu = mMenuBar->addMenu("View");

    const auto filteringMenu = mViewMenu->addMenu("Filtering");

    mNoneQuality = filteringMenu->addAction("None", [this]() {
        eFilterSettings::sSetDisplayFilter(kNone_SkFilterQuality);
        centralWidget()->update();

        mLowQuality->setChecked(false);
        mMediumQuality->setChecked(false);
        mHighQuality->setChecked(false);
        mDynamicQuality->setChecked(false);
    });
    mNoneQuality->setCheckable(true);
    mNoneQuality->setChecked(eFilterSettings::sDisplay() == kNone_SkFilterQuality &&
                             !eFilterSettings::sSmartDisplat());

    mLowQuality = filteringMenu->addAction("Low", [this]() {
        eFilterSettings::sSetDisplayFilter(kLow_SkFilterQuality);
        centralWidget()->update();

        mNoneQuality->setChecked(false);
        mMediumQuality->setChecked(false);
        mHighQuality->setChecked(false);
        mDynamicQuality->setChecked(false);
    });
    mLowQuality->setCheckable(true);
    mLowQuality->setChecked(eFilterSettings::sDisplay() == kLow_SkFilterQuality &&
                            !eFilterSettings::sSmartDisplat());

    mMediumQuality = filteringMenu->addAction("Medium", [this]() {
        eFilterSettings::sSetDisplayFilter(kMedium_SkFilterQuality);
        centralWidget()->update();

        mNoneQuality->setChecked(false);
        mLowQuality->setChecked(false);
        mHighQuality->setChecked(false);
        mDynamicQuality->setChecked(false);
    });
    mMediumQuality->setCheckable(true);
    mMediumQuality->setChecked(eFilterSettings::sDisplay() == kMedium_SkFilterQuality &&
                               !eFilterSettings::sSmartDisplat());

    mHighQuality = filteringMenu->addAction("High", [this]() {
        eFilterSettings::sSetDisplayFilter(kHigh_SkFilterQuality);
        centralWidget()->update();

        mNoneQuality->setChecked(false);
        mLowQuality->setChecked(false);
        mMediumQuality->setChecked(false);
        mDynamicQuality->setChecked(false);
    });
    mHighQuality->setCheckable(true);
    mHighQuality->setChecked(eFilterSettings::sDisplay() == kHigh_SkFilterQuality &&
                             !eFilterSettings::sSmartDisplat());

    mDynamicQuality = filteringMenu->addAction("Dynamic", [this]() {
        eFilterSettings::sSetSmartDisplay(true);
        centralWidget()->update();

        mLowQuality->setChecked(false);
        mMediumQuality->setChecked(false);
        mHighQuality->setChecked(false);
        mNoneQuality->setChecked(false);
    });
    mDynamicQuality->setCheckable(true);
    mDynamicQuality->setChecked(eFilterSettings::sSmartDisplat());

    mClipViewToCanvas = mViewMenu->addAction("Clip to Scene");
    mClipViewToCanvas->setCheckable(true);
    mClipViewToCanvas->setChecked(true);
    mClipViewToCanvas->setShortcut(QKeySequence(Qt::Key_C));
    connect(mClipViewToCanvas, &QAction::triggered,
            &mActions, &Actions::setClipToCanvas);

    mRasterEffectsVisible = mViewMenu->addAction("Raster Effects");
    mRasterEffectsVisible->setCheckable(true);
    mRasterEffectsVisible->setChecked(true);
    connect(mRasterEffectsVisible, &QAction::triggered,
            &mActions, &Actions::setRasterEffectsVisible);

    mPathEffectsVisible = mViewMenu->addAction("Path Effects");
    mPathEffectsVisible->setCheckable(true);
    mPathEffectsVisible->setChecked(true);
    connect(mPathEffectsVisible, &QAction::triggered,
            &mActions, &Actions::setPathEffectsVisible);


    mPanelsMenu = mViewMenu->addMenu("Docks");

    mSelectedObjectDockAct = mPanelsMenu->addAction("Selected Objects");
    mSelectedObjectDockAct->setCheckable(true);
    mSelectedObjectDockAct->setChecked(true);
    mSelectedObjectDockAct->setShortcut(QKeySequence(Qt::Key_O));

    connect(mSelectedObjectDock, &CloseSignalingDockWidget::madeVisible,
            mSelectedObjectDockAct, &QAction::setChecked);
    connect(mSelectedObjectDockAct, &QAction::toggled,
            mSelectedObjectDock, &QDockWidget::setVisible);

    mFilesDockAct = mPanelsMenu->addAction("Files");
    mFilesDockAct->setCheckable(true);
    mFilesDockAct->setChecked(true);
    mFilesDockAct->setShortcut(QKeySequence(Qt::Key_F));

    connect(mFilesDock, &CloseSignalingDockWidget::madeVisible,
            mFilesDockAct, &QAction::setChecked);
    connect(mFilesDockAct, &QAction::toggled,
            mFilesDock, &QDockWidget::setVisible);

    mTimelineDockAct = mPanelsMenu->addAction("Timeline");
    mTimelineDockAct->setCheckable(true);
    mTimelineDockAct->setChecked(true);
    mTimelineDockAct->setShortcut(QKeySequence(Qt::Key_T));

    connect(mTimelineDock, &CloseSignalingDockWidget::madeVisible,
            mTimelineDockAct, &QAction::setChecked);
    connect(mTimelineDockAct, &QAction::toggled,
            mTimelineDock, &QDockWidget::setVisible);

    mFillAndStrokeDockAct = mPanelsMenu->addAction("Fill and Stroke");
    mFillAndStrokeDockAct->setCheckable(true);
    mFillAndStrokeDockAct->setChecked(true);
    mFillAndStrokeDockAct->setShortcut(QKeySequence(Qt::Key_E));

    connect(mFillStrokeSettingsDock, &CloseSignalingDockWidget::madeVisible,
            mFillAndStrokeDockAct, &QAction::setChecked);
    connect(mFillAndStrokeDockAct, &QAction::toggled,
            mFillStrokeSettingsDock, &QDockWidget::setVisible);

    mBrushDockAction = mPanelsMenu->addAction("Paint Brush");
    mBrushDockAction->setCheckable(true);
    mBrushDockAction->setChecked(false);
    mBrushDockAction->setShortcut(QKeySequence(Qt::Key_B));

    connect(mBrushSettingsDock, &CloseSignalingDockWidget::madeVisible,
            mBrushDockAction, &QAction::setChecked);
    connect(mBrushDockAction, &QAction::toggled,
            mBrushSettingsDock, &QDockWidget::setVisible);

    mBrushColorBookmarksAction = mPanelsMenu->addAction("Brush/Color Bookmarks");
    mBrushColorBookmarksAction->setCheckable(true);
    mBrushColorBookmarksAction->setChecked(true);
    mBrushColorBookmarksAction->setShortcut(QKeySequence(Qt::Key_U));

    connect(mBrushColorBookmarksAction, &QAction::toggled,
            mCentralWidget, &CentralWidget::setSidesVisibilitySetting);

    const auto help = mMenuBar->addMenu("Help");

    help->addAction("License", this, [this]() {
        if(EnveLicense::sInstance) {
            delete EnveLicense::sInstance;
        } else {
            const auto license = new EnveLicense(this);
            license->show();
        }
    });

    mMenuBar->addSeparator();
    mMenuBar->addAction("Support enve", this, []() {
        QDesktopServices::openUrl(QUrl("https://maurycyliebner.github.io/"));
    });

    setMenuBar(mMenuBar);
    mMenuBar->setStyleSheet("QMenuBar { padding-top: 1px; }");
}


#include "welcomedialog.h"
void MainWindow::openWelcomeDialog() {
    if(mWelcomeDialog) return;
    mWelcomeDialog = new WelcomeDialog(getRecentFiles(),
       [this]() { CanvasSettingsDialog::sNewCanvasDialog(mDocument, this); },
       []() { MainWindow::sGetInstance()->openFile(); },
       [](QString path) { MainWindow::sGetInstance()->openFile(path); },
       this);
    takeCentralWidget();
    setCentralWidget(mWelcomeDialog);
}

void MainWindow::closeWelcomeDialog() {
    SimpleTask::sSchedule([this]() {
        if(!mWelcomeDialog) return;
        mWelcomeDialog = nullptr;
        setCentralWidget(mCentralWidget);
    });
}

void MainWindow::addCanvasToRenderQue() {
    if(!mDocument.fActiveScene) return;
    mTimeline->getRenderWidget()->
    createNewRenderInstanceWidgetForCanvas(mDocument.fActiveScene);
}

void MainWindow::updateSettingsForCurrentCanvas(Canvas* const scene) {
    mObjectSettingsWidget->setCurrentScene(scene);
    if(!scene) {
        mObjectSettingsWidget->setMainTarget(nullptr);
        mTimeline->updateSettingsForCurrentCanvas(nullptr);
        return;
    }
    mClipViewToCanvas->setChecked(scene->clipToCanvas());
    mRasterEffectsVisible->setChecked(scene->getRasterEffectsVisible());
    mPathEffectsVisible->setChecked(scene->getPathEffectsVisible());
    mTimeline->updateSettingsForCurrentCanvas(scene);
    mObjectSettingsWidget->setMainTarget(scene->getCurrentGroup());
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

    const QString iconsDir = eSettings::sIconsDir() + "/toolbarButtons";

    mBoxTransformMode = SwitchButton::sCreate2Switch(
                iconsDir + "/boxTransformUnchecked.png",
                iconsDir + "/boxTransformChecked.png",
                gSingleLineTooltip("Object Mode", "F1"), this);
    mBoxTransformMode->toggle();
    mToolBar->addWidget(mBoxTransformMode);

    mPointTransformMode = SwitchButton::sCreate2Switch(
                iconsDir + "/pointTransformUnchecked.png",
                iconsDir + "/pointTransformChecked.png",
                gSingleLineTooltip("Point Mode", "F2"), this);
    mToolBar->addWidget(mPointTransformMode);

    mAddPointMode = SwitchButton::sCreate2Switch(
                iconsDir + "/pathCreateUnchecked.png",
                iconsDir +  "/pathCreateChecked.png",
                gSingleLineTooltip("Add Path Mode", "F3"), this);
    mToolBar->addWidget(mAddPointMode);

    mPaintMode = SwitchButton::sCreate2Switch(
                iconsDir + "/paintUnchecked.png",
                iconsDir + "/paintChecked.png",
                gSingleLineTooltip("Paint Mode", "F4"), this);
    mToolBar->addWidget(mPaintMode);

    mToolBar->addSeparator();

    mSculptMode = SwitchButton::sCreate2Switch(
                iconsDir + "/sculptUnchecked.png",
                iconsDir + "/sculptChecked.png",
                gSingleLineTooltip("Sculpt Path Mode", "F5"), this);
    mToolBar->addWidget(mSculptMode);

    mCircleMode = SwitchButton::sCreate2Switch(
                iconsDir + "/circleCreateUnchecked.png",
                iconsDir + "/circleCreateChecked.png",
                gSingleLineTooltip("Add Circle Mode", "F6"), this);
    mToolBar->addWidget(mCircleMode);

    mRectangleMode = SwitchButton::sCreate2Switch(
                iconsDir + "/rectCreateUnchecked.png",
                iconsDir + "/rectCreateChecked.png",
                gSingleLineTooltip("Add Rectange Mode", "F7"), this);
    mToolBar->addWidget(mRectangleMode);

    mTextMode = SwitchButton::sCreate2Switch(
                iconsDir + "/textCreateUnchecked.png",
                iconsDir + "/textCreateChecked.png",
                gSingleLineTooltip("Add Text Mode", "F8"), this);
    mToolBar->addWidget(mTextMode);

    mToolBar->addSeparator();

    mPickPaintSettingsMode = SwitchButton::sCreate2Switch(
                iconsDir + "/pickUnchecked.png",
                iconsDir + "/pickChecked.png",
                gSingleLineTooltip("Pick Mode", "F9"), this);
    mToolBar->addWidget(mPickPaintSettingsMode);

    mToolBar->widgetForAction(mToolBar->addAction("     "))->
            setObjectName("emptyToolButton");
    mToolBar->addWidget(mLayoutHandler->comboWidget());
    mToolBar->widgetForAction(mToolBar->addAction("     "))->
            setObjectName("emptyToolButton");

    mActionConnectPoints = new ActionButton(iconsDir + "/nodeConnect.png",
                                            "Connect Nodes", this);
    mActionConnectPointsAct = mToolBar->addWidget(mActionConnectPoints);

    mActionDisconnectPoints = new ActionButton(iconsDir + "/nodeDisconnect.png",
                                               "Disconnect Nodes", this);
    mActionDisconnectPointsAct = mToolBar->addWidget(mActionDisconnectPoints);

    mActionMergePoints = new ActionButton(iconsDir + "/nodeMerge.png",
                                          "Merge Nodes", this);
    mActionMergePointsAct = mToolBar->addWidget(mActionMergePoints);

    mActionNewNode = new ActionButton(iconsDir + "/nodeNew.png",
                                      "New Node", this);
    mActionNewNodeAct = mToolBar->addWidget(mActionNewNode);
//
    mSeparator1 = mToolBar->addSeparator();

    mActionSymmetricPointCtrls = new ActionButton(iconsDir + "/nodeSymmetric.png",
                                                  "Symmetric Nodes", this);
    mActionSymmetricPointCtrlsAct = mToolBar->addWidget(mActionSymmetricPointCtrls);

    mActionSmoothPointCtrls = new ActionButton(iconsDir + "/nodeSmooth.png",
                                               "Smooth Nodes", this);
    mActionSmoothPointCtrlsAct = mToolBar->addWidget(mActionSmoothPointCtrls);

    mActionCornerPointCtrls = new ActionButton(iconsDir + "/nodeCorner.png",
                                               "Corner Nodes", this);
    mActionCornerPointCtrlsAct = mToolBar->addWidget(mActionCornerPointCtrls);

//
    mSeparator2 = mToolBar->addSeparator();

    mActionLine = new ActionButton(iconsDir + "/segmentLine.png",
                                   gSingleLineTooltip("Make Segment Line"), this);
    mActionLineAct = mToolBar->addWidget(mActionLine);

    mActionCurve = new ActionButton(iconsDir + "/segmentCurve.png",
                                    gSingleLineTooltip("Make Segment Curve"), this);
    mActionCurveAct = mToolBar->addWidget(mActionCurve);

    mFontWidget = new FontsWidget(this);
    mFontWidgetAct = mToolBar->addWidget(mFontWidget);

    mActionNewEmptyPaintFrame = new ActionButton(
                iconsDir + "/newEmpty.png",
                gSingleLineTooltip("New Empty Frame", "N"), this);
    mActionNewEmptyPaintFrameAct = mToolBar->addWidget(mActionNewEmptyPaintFrame);

    mToolBar->setFixedHeight(2*MIN_WIDGET_DIM);

    addToolBar(mToolBar);

    mDocument.setCanvasMode(CanvasMode::boxTransform);
}

void MainWindow::connectToolBarActions() {
    connect(mBoxTransformMode, &ActionButton::pressed,
            &mActions, &Actions::setMovePathMode);
    connect(mPointTransformMode, &ActionButton::pressed,
            &mActions, &Actions::setMovePointMode);
    connect(mAddPointMode, &ActionButton::pressed,
            &mActions, &Actions::setAddPointMode);
    connect(mPaintMode, &ActionButton::pressed,
            &mActions, &Actions::setPaintMode);

    connect(mSculptMode, &ActionButton::pressed,
            &mActions, &Actions::setSculptMode);
    connect(mCircleMode, &ActionButton::pressed,
            &mActions, &Actions::setCircleMode);
    connect(mRectangleMode, &ActionButton::pressed,
            &mActions, &Actions::setRectangleMode);
    connect(mTextMode, &ActionButton::pressed,
            &mActions, &Actions::setTextMode);

    connect(mPickPaintSettingsMode, &ActionButton::pressed,
            &mActions, &Actions::setPickPaintSettingsMode);

    connect(mActionConnectPoints, &ActionButton::pressed,
            &mActions, &Actions::connectPointsSlot);
    connect(mActionDisconnectPoints, &ActionButton::pressed,
            &mActions, &Actions::disconnectPointsSlot);
    connect(mActionMergePoints, &ActionButton::pressed,
            &mActions, &Actions::mergePointsSlot);
    connect(mActionNewNode, &ActionButton::pressed,
            &mActions, &Actions::subdivideSegments);

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
    connect(mFontWidget, &FontsWidget::textAlignmentChanged,
            &mActions, &Actions::setTextAlignment);
    connect(mFontWidget, &FontsWidget::textVAlignmentChanged,
            &mActions, &Actions::setTextVAlignment);

    connect(mActionNewEmptyPaintFrame, &ActionButton::pressed,
            &mActions, &Actions::newEmptyPaintFrame);
}

MainWindow *MainWindow::sGetInstance() {
    return sInstance;
}

void MainWindow::updateCanvasModeButtonsChecked() {
    const CanvasMode mode = mDocument.fCanvasMode;
    mCentralWidget->setCanvasMode(mode);

    mBoxTransformMode->setState(mode == CanvasMode::boxTransform);
    mPointTransformMode->setState(mode == CanvasMode::pointTransform);
    mAddPointMode->setState(mode == CanvasMode::pathCreate);
    mPaintMode->setState(mode == CanvasMode::paint);

    mSculptMode->setState(mode == CanvasMode::sculptPath);
    mCircleMode->setState(mode == CanvasMode::circleCreate);
    mRectangleMode->setState(mode == CanvasMode::rectCreate);
    mTextMode->setState(mode == CanvasMode::textCreate);

    mPickPaintSettingsMode->setState(mode == CanvasMode::pickFillStroke);

    const bool boxMode = mode == CanvasMode::boxTransform;
    mFontWidgetAct->setVisible(boxMode);

    const bool pointMode = mode == CanvasMode::pointTransform;
    mActionConnectPointsAct->setVisible(pointMode);
    mActionDisconnectPointsAct->setVisible(pointMode);
    mActionMergePointsAct->setVisible(pointMode);
    mActionNewNodeAct->setVisible(pointMode);
    mActionSymmetricPointCtrlsAct->setVisible(pointMode);
    mActionSmoothPointCtrlsAct->setVisible(pointMode);
    mActionCornerPointCtrlsAct->setVisible(pointMode);
    mActionLineAct->setVisible(pointMode);
    mActionCurveAct->setVisible(pointMode);
    mSeparator1->setVisible(pointMode);
    mSeparator2->setVisible(pointMode);

    const bool paintMode = mode == CanvasMode::paint;
    mActionNewEmptyPaintFrameAct->setVisible(paintMode);
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
    if(!mDocument.fActiveScene) return;
    mDocument.fActiveScene->setResolutionFraction(value);
    mDocument.actionFinished();
}

void MainWindow::setFileChangedSinceSaving(const bool changed) {
    if(changed == mChangedSinceSaving) return;
    mChangedSinceSaving = changed;
    updateTitle();
}

SimpleBrushWrapper *MainWindow::getCurrentBrush() const {
    return mBrushSelectionWidget->getCurrentBrush();
}

#include "Boxes/textbox.h"
void MainWindow::setCurrentBox(BoundingBox *box) {
    mFillStrokeSettings->setCurrentBox(box);
    if(box && box->SWT_isTextBox()) {
        const auto txtBox = static_cast<TextBox*>(box);
        mFontWidget->setCurrentSettings(txtBox->getFontSize(),
                                        txtBox->getFontFamily(),
                                        txtBox->getFontStyle());
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
        document.setCanvasMode(CanvasMode::boxTransform);
    } else if(key == Qt::Key_F2) {
        document.setCanvasMode(CanvasMode::pointTransform);
    } else if(key == Qt::Key_F3) {
        document.setCanvasMode(CanvasMode::pathCreate);
    }  else if(key == Qt::Key_F4) {
        document.setCanvasMode(CanvasMode::paint);
    } else if(key == Qt::Key_F5) {
        document.setCanvasMode(CanvasMode::sculptPath);
    } else if(key == Qt::Key_F6) {
        document.setCanvasMode(CanvasMode::circleCreate);
    } else if(key == Qt::Key_F7) {
        document.setCanvasMode(CanvasMode::rectCreate);
    } else if(key == Qt::Key_F8) {
        document.setCanvasMode(CanvasMode::textCreate);
    } else if(key == Qt::Key_F9) {
        document.setCanvasMode(CanvasMode::pickFillStroke);
    } else return false;
    KeyFocusTarget::KFT_sSetRandomTarget();
    return true;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e) {
    if(mLock) if(dynamic_cast<QInputEvent*>(e)) return true;
    if(mEventFilterDisabled) return QMainWindow::eventFilter(obj, e);
    const auto type = e->type();
    const auto focusWidget = QApplication::focusWidget();
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
        if(key == Qt::Key_Tab) {
            KeyFocusTarget::KFT_sTab();
            return true;
        }
        if(handleCanvasModeKeyPress(mDocument, key)) return true;
        if(keyEvent->modifiers() & Qt::SHIFT && key == Qt::Key_D) {
            return processKeyEvent(keyEvent);
        }
        if(keyEvent->modifiers() & Qt::CTRL) {
            if(key == Qt::Key_C || key == Qt::Key_V ||
               key == Qt::Key_X || key == Qt::Key_D) {
                return processKeyEvent(keyEvent);
            }
        } else if(key == Qt::Key_A || key == Qt::Key_I ||
                  key == Qt::Key_Delete) {
              return processKeyEvent(keyEvent);
        }
    } else if(type == QEvent::KeyRelease) {
        const auto keyEvent = static_cast<QKeyEvent*>(e);
        if(processKeyEvent(keyEvent)) return true;
        //finishUndoRedoSet();
    } else if(type == QEvent::MouseButtonRelease) {
        //finishUndoRedoSet();
    }
    return QMainWindow::eventFilter(obj, e);
}

void MainWindow::closeEvent(QCloseEvent *e) {
    if(!closeProject()) e->ignore();
}

bool MainWindow::processKeyEvent(QKeyEvent *event) {
    if(isActiveWindow()) {
        bool returnBool = false;
        if(event->type() == QEvent::KeyPress &&
            mTimeline->processKeyPress(event)) {
            returnBool = true;
        } else {
            returnBool = KeyFocusTarget::KFT_handleKeyEvent(event);
        }
        mDocument.actionFinished();
        return returnBool;
    }
    return false;
}

bool MainWindow::isEnabled() {
    return !mGrayOutWidget;
}

void MainWindow::clearAll() {
    TaskScheduler::sInstance->clearTasks();
    setFileChangedSinceSaving(false);
    mObjectSettingsWidget->setMainTarget(nullptr);

    mTimeline->clearAll();
    mFillStrokeSettings->clearAll();
    mDocument.clear();
    mLayoutHandler->clear();
//    for(ClipboardContainer *cont : mClipboardContainers) {
//        delete cont;
//    }
//    mClipboardContainers.clear();
    FilesHandler::sInstance->clear();
    //mBoxListWidget->clearAll();
    openWelcomeDialog();
}

void MainWindow::updateTitle() {
    QString star = "";
    if(mChangedSinceSaving) star = "*";
    setWindowTitle(mDocument.fEvFile.split("/").last() + star + " - enve");
}

void MainWindow::openFile() {
    if(askForSaving()) {
        disable();
        const QString defPath = mDocument.fEvFile.isEmpty() ?
                    QDir::homePath() : mDocument.fEvFile;
        const QString openPath = eDialogs::openFile("Open File", defPath,
                                                    "enve Files (*.ev)");
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
    mDocument.actionFinished();
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
    const QString defPath = mDocument.fEvFile.isEmpty() ?
                QDir::homePath() : mDocument.fEvFile;

    QString saveAs = eDialogs::saveFile("Save File", defPath,
                                        "enve Files (*.ev)");
    enableEventFilter();
    if(!saveAs.isEmpty()) {
        if(saveAs.right(3) != ".ev") saveAs += ".ev";
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
        clearAll();
        return true;
    }
    return false;
}

void MainWindow::importFile() {
    disableEventFilter();
    const QString defPath = mDocument.fEvFile.isEmpty() ?
                QDir::homePath() : mDocument.fEvFile;
    const auto importPaths = eDialogs::openFiles(
                "Import File", defPath,
                "Files (*.ev *.svg " +
                        FileExtensions::videoFilters() +
                        FileExtensions::imageFilters() +
                        FileExtensions::soundFilters() + ")");
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
    const QString defPath = mDocument.fEvFile.isEmpty() ?
                QDir::homePath() : mDocument.fEvFile;
    QStringList importPaths = eDialogs::openFiles(
        "Link File", defPath, "enve Files (*.ev)");
    enableEventFilter();
    if(!importPaths.isEmpty()) {
        for(const QString &path : importPaths) {
            if(path.isEmpty()) continue;
            mActions.linkFile(path);
        }
    }
}

void MainWindow::importImageSequence() {
    disableEventFilter();
    const QString defPath = mDocument.fEvFile.isEmpty() ?
                QDir::homePath() : mDocument.fEvFile;
    const auto folder = eDialogs::openDir("Import Image Sequence", defPath);
    enableEventFilter();
    if(!folder.isEmpty()) mActions.importFile(folder);
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
    const auto path = mDocument.fEvFile;
    clearAll();
    try {
        loadEVFile(path);
        mDocument.setPath(path);
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
    }
    setFileChangedSinceSaving(false);
    mDocument.actionFinished();
}

stdsptr<void> MainWindow::lock() {
    if(mLock) return mLock->ref<Lock>();
    setEnabled(false);
    const auto newLock = enve::make_shared<Lock>(this);
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
