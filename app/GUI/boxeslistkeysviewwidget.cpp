#include "boxeslistkeysviewwidget.h"
#include "animationwidgetscrollbar.h"
#include "mainwindow.h"
#include "boxeslistanimationdockwidget.h"
#include "GUI/BoxesList/boxsinglewidget.h"
#include "singlewidgetabstraction.h"
#include "GUI/BoxesList/boxscrollwidget.h"
#include "keysview.h"
#include "GUI/BoxesList/boxscrollarea.h"
#include "GUI/BoxesList/boxscrollwidgetvisiblepart.h"
#include "canvaswindow.h"
#include "animationdockwidget.h"
#include "global.h"
#include "canvas.h"
#include "scenechooser.h"
#include "changewidthwidget.h"
#include <QToolButton>

TimelineWidget::TimelineWidget(Document &document,
                               StackWrapperCornerMenu * const menu,
                               QWidget *parent) :
    QWidget(parent), mDocument(document) {
    mMainWindow = MainWindow::getInstance();

    mMainLayout = new QGridLayout(this);
    mMainLayout->setSpacing(0);
    mMainLayout->setMargin(0);

    mMenuLayout = new QHBoxLayout();
    mMenuLayout->setSpacing(0);
    mMenuLayout->setMargin(0);
    mBoxesListMenuBar = new QMenuBar(this);
    mBoxesListMenuBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    ((QToolButton*)mBoxesListMenuBar->children()[0])->setStyleSheet(
                "QToolButton {"
                    "padding: 0px 0px;"
                    "background: transparent;"
                    "border-radius: 4px;"
                    "margin-top: 0;"
                    "border-bottom-right-radius: 0px;"
                    "border-bottom-left-radius: 0px;"
                    "padding-bottom: 0;"
                    "margin-bottom: 0;"
                "}"
                "QToolButton:hover {"
                    "background-color: rgba(0, 0, 0, 30);"
                "}"

                "QToolButton:pressed {"
                    "background-color: rgba(0, 0, 0, 50);"
                "}"

                "QToolButton:checked {"
                    "background-color: rgb(60, 60, 60);"
                    "color: white;"
                "}");
    mBoxesListMenuBar->addSeparator();
    mSceneChooser = new SceneChooser(mDocument, true,
                                     mBoxesListMenuBar);
    mBoxesListMenuBar->addMenu(mSceneChooser);

    mCornerMenuBar = new QMenuBar(this);
    mCornerMenuBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    mCornerMenuBar->setStyleSheet("QMenuBar::item { padding: 1px 0px; }");

    QMenu * const settingsMenu = mCornerMenuBar->addMenu(
                QIcon(":/icons/settings_dots.png"), "Settings");
    QMenu * const objectsMenu = settingsMenu->addMenu("State");
    objectsMenu->addAction("All", this,
                           &TimelineWidget::setRuleNone);
    objectsMenu->addAction("Selected", this,
                           &TimelineWidget::setRuleSelected);
    objectsMenu->addAction("Animated", this,
                           &TimelineWidget::setRuleAnimated);
    objectsMenu->addAction("Not Animated", this,
                           &TimelineWidget::setRuleNotAnimated);
    objectsMenu->addAction("Visible", this,
                           &TimelineWidget::setRuleVisible);
    objectsMenu->addAction("Hidden", this,
                           &TimelineWidget::setRuleHidden);
    objectsMenu->addAction("Unlocked", this,
                           &TimelineWidget::setRuleUnloced);
    objectsMenu->addAction("Locked", this,
                           &TimelineWidget::setRuleLocked);

    QMenu * const targetMenu = settingsMenu->addMenu("Target");
//    targetMenu->addAction("All", this,
//                          &BoxesListKeysViewWidget::setTargetAll);
    targetMenu->addAction("Current Canvas", this,
                          &TimelineWidget::setTargetCurrentCanvas);
    targetMenu->addAction("Current Group", this,
                          &TimelineWidget::setTargetCurrentGroup);
    QMenu * const typeMenu = settingsMenu->addMenu("Type");
    typeMenu->addAction("All", this, &TimelineWidget::setTypeAll);
    typeMenu->addAction("Graphics", this,
                        &TimelineWidget::setTypeGraphics);
    typeMenu->addAction("Sound", this,
                        &TimelineWidget::setTypeSound);

    //QMenu *viewMenu = mBoxesListMenuBar->addMenu("View");
    mGraphAct = mCornerMenuBar->addAction("Graph");
    mGraphAct->setIcon(QIcon(":/icons/graphDisabled.png"));
    mGraphAct->setCheckable(true);
    connect(mGraphAct, &QAction::toggled,
            this, &TimelineWidget::setGraphEnabled);

    mCornerMenuBar->setCornerWidget(menu);
//    mCornerMenuBar->addSeparator();
//    mCornerMenuBar->addAction(" + ", this,
//                              &TimelineWidget::addNewBelowThis);
//    mCornerMenuBar->addAction(" - ", this,
//                              &TimelineWidget::removeThis);
//    mCornerMenuBar->addSeparator();

    mSearchLine = new QLineEdit("", mBoxesListMenuBar);
    mSearchLine->setMinimumHeight(0);
    mSearchLine->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    mSearchLine->setProperty("forceHandleEvent", QVariant(true));
    mSearchLine->setStyleSheet("background-color: white;"
                               "color: black;"
                               "border-radius: 0;"
                               "border: 0;"
                               "border-right: 1px solid black;"
                               "border-left: 1px solid black;"
                               "border-bottom: 1px solid black;"
                               "margin: 0;");
    connect(mSearchLine, &QLineEdit::textChanged,
            this, &TimelineWidget::setSearchText);

    mMenuLayout->addWidget(mBoxesListMenuBar);
    mMenuLayout->addWidget(mSearchLine);
    mMenuLayout->addWidget(mCornerMenuBar);

    mMenuWidget = new QWidget(this);
    mMenuWidget->setLayout(mMenuLayout);

    mBoxesListScrollArea = new ScrollArea(this);

    mBoxesListWidget = new BoxScrollWidget(mDocument, mBoxesListScrollArea);
    auto visiblePartWidget = mBoxesListWidget->getVisiblePartWidget();
    visiblePartWidget->setCurrentRule(SWT_BR_ALL);
    visiblePartWidget->setCurrentTarget(nullptr, SWT_TARGET_CURRENT_CANVAS);

    mBoxesListScrollArea->setWidget(mBoxesListWidget);
    mMainLayout->addWidget(mMenuWidget, 0, 0);
    mMainLayout->addWidget(mBoxesListScrollArea, 1, 0);

    mKeysViewLayout = new QVBoxLayout();
    mKeysView = new KeysView(mBoxesListWidget->getVisiblePartWidget(), this);
    mKeysViewLayout->addWidget(mKeysView);
    mAnimationDockWidget = new AnimationDockWidget(this, mKeysView);
    mAnimationDockWidget->hide();
    mMainLayout->addLayout(mKeysViewLayout, 1, 1);

    const auto keysViewScrollbarLayout = new QHBoxLayout();
    const auto layoutT = new QVBoxLayout();
    layoutT->setAlignment(Qt::AlignBottom);
    layoutT->addWidget(mAnimationDockWidget);
    keysViewScrollbarLayout->addLayout(layoutT);
    mKeysView->setLayout(keysViewScrollbarLayout);
    keysViewScrollbarLayout->setAlignment(Qt::AlignRight);
    keysViewScrollbarLayout->addWidget(
                mBoxesListScrollArea->verticalScrollBar());
    mBoxesListScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    keysViewScrollbarLayout->setContentsMargins(0, 0, 0, 0);

    connect(mBoxesListScrollArea->verticalScrollBar(),
            &QScrollBar::valueChanged,
            mBoxesListWidget, &BoxScrollWidget::changeVisibleTop);
    connect(mBoxesListScrollArea, &ScrollArea::heightChanged,
            mBoxesListWidget, &BoxScrollWidget::changeVisibleHeight);
    connect(mBoxesListScrollArea, &ScrollArea::widthChanged,
            mBoxesListWidget, &BoxScrollWidget::setWidth);

    connect(mBoxesListScrollArea->verticalScrollBar(),
            &QScrollBar::valueChanged,
            this, &TimelineWidget::moveSlider);
    connect(mKeysView, &KeysView::wheelEventSignal,
            mBoxesListScrollArea, &ScrollArea::callWheelEvent);

    connect(mSceneChooser, &SceneChooser::currentChanged,
            this, &TimelineWidget::setCurrentScene);

    mBoxesListScrollArea->setFixedWidth(20*MIN_WIDGET_DIM);

    setLayout(mMainLayout);

    mFrameScrollBar = new FrameScrollBar(1, 1, 0,
                                         false, false, this);
    mFrameScrollBar->setSizePolicy(QSizePolicy::Minimum,
                                   QSizePolicy::Preferred);
//    connect(MemoryHandler::sGetInstance(), &MemoryHandler::memoryFreed,
//            frameScrollBar,
//            qOverload<>(&FrameScrollBar::update));
    connect(mFrameScrollBar, &FrameScrollBar::viewedFrameRangeChanged,
            this, [this](const FrameRange& range){
        const auto scene = mSceneChooser->getCurrentScene();
        if(scene) scene->anim_setAbsFrame(range.fMin);
        MainWindow::getInstance()->queScheduledTasksAndUpdate();
    });
    mMainLayout->addWidget(mFrameScrollBar, 0, 1);

    mFrameRangeScrollBar = new FrameScrollBar(20, 200, MIN_WIDGET_DIM*2/3,
                                              true, true, this);

    connect(mFrameRangeScrollBar, &FrameScrollBar::viewedFrameRangeChanged,
            this, &TimelineWidget::setViewedFrameRange);
    mKeysViewLayout->addWidget(mFrameRangeScrollBar);
    mSceneChooser->setCurrentScene(mDocument.fActiveScene);


    const auto chww = new ChangeWidthWidget(this);
    chww->show();
    chww->updatePos();
    chww->raise();
    connect(chww, &ChangeWidthWidget::widthSet,
            this, &TimelineWidget::setBoxesListWidth);
    setBoxesListWidth(chww->getCurrentWidth());
}

void TimelineWidget::setCurrentScene(Canvas * const scene) {
    if(scene == mCurrentScene) return;
    if(mCurrentScene) {
        disconnect(mCurrentScene, nullptr, mFrameScrollBar, nullptr);
        disconnect(mCurrentScene, nullptr, this, nullptr);
    }

    mCurrentScene = scene;
    mSceneChooser->setCurrentScene(scene);
    mFrameScrollBar->setCurrentCanvas(scene);
    mKeysView->setCurrentScene(scene);
    if(scene) {
        connect(scene, &Canvas::currentFrameChanged,
                mFrameScrollBar, &FrameScrollBar::setFirstViewedFrame);
        connect(scene, &Canvas::newFrameRange,
                this, &TimelineWidget::setCanvasFrameRange);
    }
}

void TimelineWidget::setGraphEnabled(const bool enabled) {
    mKeysView->setGraphViewed(enabled);
    mAnimationDockWidget->setVisible(enabled);
    if(enabled) mGraphAct->setIcon(QIcon(":/icons/graphEnabled.png"));
    else mGraphAct->setIcon(QIcon(":/icons/graphDisabled.png"));
}

void TimelineWidget::moveSlider(int val) {
    int diff = val%MIN_WIDGET_DIM;
    if(diff != 0) {
        val -= diff;
        mBoxesListScrollArea->verticalScrollBar()->setSliderPosition(val);
    }
    mKeysView->setViewedVerticalRange(
                val, val + mBoxesListScrollArea->height());
}

void TimelineWidget::setBoxesListWidth(const int width) {
    mMenuWidget->setFixedWidth(width);
    mBoxesListScrollArea->setFixedWidth(width);
}

void TimelineWidget::setBoxRule(const SWT_BoxRule rule) {
    mBoxesListWidget->getVisiblePartWidget()->setCurrentRule(rule);
    mMainWindow->queScheduledTasksAndUpdate();
}

void TimelineWidget::setRuleNone() {
    setBoxRule(SWT_BR_ALL);
}

void TimelineWidget::setRuleSelected() {
    setBoxRule(SWT_BR_SELECTED);
}

void TimelineWidget::setRuleAnimated() {
    setBoxRule(SWT_BR_ANIMATED);
}

void TimelineWidget::setRuleNotAnimated() {
    setBoxRule(SWT_BR_NOT_ANIMATED);
}

void TimelineWidget::setRuleVisible() {
    setBoxRule(SWT_BR_VISIBLE);
}

void TimelineWidget::setRuleHidden() {
    setBoxRule(SWT_BR_HIDDEN);
}

void TimelineWidget::setRuleUnloced() {
    setBoxRule(SWT_BR_UNLOCKED);
}

void TimelineWidget::setRuleLocked() {
    setBoxRule(SWT_BR_LOCKED);
}

void TimelineWidget::setTargetAll() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(&mDocument, SWT_TARGET_ALL);
    mMainWindow->queScheduledTasksAndUpdate();
}

void TimelineWidget::setTargetCurrentCanvas() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(
                mCurrentScene,
                SWT_TARGET_CURRENT_CANVAS);
    mMainWindow->queScheduledTasksAndUpdate();
}

void TimelineWidget::setTargetCurrentGroup() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(
                mCurrentScene->getCurrentGroup(),
                SWT_TARGET_CURRENT_GROUP);
    mMainWindow->queScheduledTasksAndUpdate();
}

void TimelineWidget::setCurrentType(const SWT_Type type) {
    mBoxesListWidget->getVisiblePartWidget()->setCurrentType(type);
    mMainWindow->queScheduledTasksAndUpdate();
}

void TimelineWidget::setTypeAll() {
    setCurrentType(SWT_TYPE_ALL);
}

void TimelineWidget::setTypeSound() {
    setCurrentType(SWT_TYPE_SOUND);
}

void TimelineWidget::setTypeGraphics() {
    setCurrentType(SWT_TYPE_GRAPHICS);
}

void TimelineWidget::setSearchText(const QString &text) {
    mBoxesListWidget->getVisiblePartWidget()->setCurrentSearchText(text);
    mMainWindow->queScheduledTasksAndUpdate();
}

void TimelineWidget::setViewedFrameRange(
        const FrameRange& range) {
    mFrameRangeScrollBar->setViewedFrameRange(range);
    mFrameScrollBar->setDisplayedFrameRange(range);
    mKeysView->setFramesRange(range);
}

void TimelineWidget::setCanvasFrameRange(
        const FrameRange& range) {
    mFrameRangeScrollBar->setDisplayedFrameRange(range);
    setViewedFrameRange(mFrameRangeScrollBar->getViewedRange());
    mFrameRangeScrollBar->setCanvasFrameRange(range);
    mFrameScrollBar->setCanvasFrameRange(range);
}
