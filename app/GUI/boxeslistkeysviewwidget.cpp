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
#include <QToolButton>

BoxesListKeysViewWidget::BoxesListKeysViewWidget(Document &document,
                            BoxesListAnimationDockWidget *animationDock,
                            QWidget *parent) :
    QWidget(parent), mDocument(document) {
    mMainWindow = MainWindow::getInstance();
    mBoxesListAnimationDockWidget = animationDock;

    mMainLayout = new QHBoxLayout(this);
    mMainLayout->setSpacing(0);
    mMainLayout->setMargin(0);

    mMenuLayout = new QHBoxLayout();
    mMenuLayout->setSpacing(0);
    mMenuLayout->setMargin(0);
    mBoxesListMenuBar = new QMenuBar(this);
    mBoxesListMenuBar->setFixedHeight(MIN_WIDGET_DIM);
    mBoxesListMenuBar->setStyleSheet("QMenuBar {"
                                        "border-top: 0;"
                                        "border-bottom: 1px solid black;"
                                     "}");
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
    const auto sceneChooser = new SceneChooser(mDocument, true,
                                               mBoxesListMenuBar);
    mBoxesListMenuBar->addMenu(sceneChooser);

    mCornerMenuBar = new QMenuBar(this);
    mCornerMenuBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
    mCornerMenuBar->setFixedHeight(MIN_WIDGET_DIM);
    mCornerMenuBar->setStyleSheet("QMenuBar {"
                                     "border-top: 0;"
                                     "border-bottom: 1px solid black;"
                                  "}");

    QMenu * const settingsMenu = mCornerMenuBar->addMenu("o");
    QMenu * const objectsMenu = settingsMenu->addMenu("State");
    objectsMenu->addAction("All", this,
                           &BoxesListKeysViewWidget::setRuleNone);
    objectsMenu->addAction("Selected", this,
                           &BoxesListKeysViewWidget::setRuleSelected);
    objectsMenu->addAction("Animated", this,
                           &BoxesListKeysViewWidget::setRuleAnimated);
    objectsMenu->addAction("Not Animated", this,
                           &BoxesListKeysViewWidget::setRuleNotAnimated);
    objectsMenu->addAction("Visible", this,
                           &BoxesListKeysViewWidget::setRuleVisible);
    objectsMenu->addAction("Hidden", this,
                           &BoxesListKeysViewWidget::setRuleHidden);
    objectsMenu->addAction("Unlocked", this,
                           &BoxesListKeysViewWidget::setRuleUnloced);
    objectsMenu->addAction("Locked", this,
                           &BoxesListKeysViewWidget::setRuleLocked);

    QMenu * const targetMenu = settingsMenu->addMenu("Target");
//    targetMenu->addAction("All", this,
//                          &BoxesListKeysViewWidget::setTargetAll);
    targetMenu->addAction("Current Canvas", this,
                          &BoxesListKeysViewWidget::setTargetCurrentCanvas);
    targetMenu->addAction("Current Group", this,
                          &BoxesListKeysViewWidget::setTargetCurrentGroup);
    QMenu * const typeMenu = settingsMenu->addMenu("Type");
    typeMenu->addAction("All", this, &BoxesListKeysViewWidget::setTypeAll);
    typeMenu->addAction("Graphics", this,
                        &BoxesListKeysViewWidget::setTypeGraphics);
    typeMenu->addAction("Sound", this,
                        &BoxesListKeysViewWidget::setTypeSound);

    //QMenu *viewMenu = mBoxesListMenuBar->addMenu("View");
    mGraphAct = mCornerMenuBar->addAction("/");
    mGraphAct->setCheckable(true);
    connect(mGraphAct, &QAction::toggled,
            this, &BoxesListKeysViewWidget::setGraphEnabled);

    mCornerMenuBar->addSeparator();
    mCornerMenuBar->addAction(" + ", this,
                              &BoxesListKeysViewWidget::addNewBelowThis);
    mCornerMenuBar->addAction(" - ", this,
                              &BoxesListKeysViewWidget::removeThis);
    mCornerMenuBar->addSeparator();

    mMenuWidgetsLayout = new QHBoxLayout();
    mMenuWidgetsLayout->setAlignment(Qt::AlignRight);
    mMenuWidgetsLayout->setMargin(0);
    mMenuWidgetsLayout->setSpacing(0);
    mSearchLine = new QLineEdit("", mBoxesListMenuBar);
    mSearchLine->setFixedHeight(MIN_WIDGET_DIM);
    mSearchLine->setProperty("forceHandleEvent", QVariant(true));
    mSearchLine->setStyleSheet("background-color: rgb(255, 255, 255);"
                               "border-bottom: 0;"
                               "border-radius: 4px;"
                               "border: 1px solid rgb(100, 100, 100);"
                               "margin-top: 1px;"
                               "margin-bottom: 1px;");
    connect(mSearchLine, &QLineEdit::textChanged,
            this, &BoxesListKeysViewWidget::setSearchText);
    mSearchLine->setSizePolicy(QSizePolicy::Maximum,
                               QSizePolicy::Fixed);

    mBoxesListMenuBar->setSizePolicy(QSizePolicy::Minimum,
                                     QSizePolicy::Fixed);

    mMenuLayout->addWidget(mBoxesListMenuBar);

    mMenuWidgetsLayout->addWidget(mSearchLine);
    mMenuWidgetsCont = new QWidget(this);
    mMenuWidgetsCont->setStyleSheet(
                "QWidget {"
                    "border-top: 0;"
                    "border-bottom: 1px solid black;"
                    "color: black;"
                    "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
                    "stop:0 lightgray, stop:1 darkgray);"
                "}");
    mMenuWidgetsCont->setLayout(mMenuWidgetsLayout);

    mMenuLayout->addWidget(mMenuWidgetsCont);
    mMenuLayout->addWidget(mCornerMenuBar);

    mBoxesListLayout = new QVBoxLayout();
    mBoxesListLayout->setSpacing(0);
    mBoxesListLayout->setMargin(0);
    mBoxesListLayout->addLayout(mMenuLayout);

    mBoxesListScrollArea = new BoxScrollArea(this);

    mBoxesListWidget = new BoxScrollWidget(mDocument, mBoxesListScrollArea);
    auto visiblePartWidget = mBoxesListWidget->getVisiblePartWidget();
    visiblePartWidget->setCurrentRule(SWT_BR_ALL);
    visiblePartWidget->setCurrentTarget(nullptr, SWT_TARGET_CURRENT_CANVAS);

    mBoxesListScrollArea->setWidget(mBoxesListWidget);
    mBoxesListLayout->addWidget(mBoxesListScrollArea);
    mMainLayout->addLayout(mBoxesListLayout);

    mKeysViewLayout = new QVBoxLayout();
    mKeysView = new KeysView(mBoxesListWidget->getVisiblePartWidget(), this);
    mKeysViewLayout->addWidget(mKeysView);
    mAnimationDockWidget = new AnimationDockWidget(this, mKeysView);
    mAnimationDockWidget->hide();
    mMainLayout->addLayout(mKeysViewLayout);

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
            this, &BoxesListKeysViewWidget::moveSlider);
    connect(mKeysView, &KeysView::wheelEventSignal,
            mBoxesListScrollArea, &ScrollArea::callWheelEvent);

    connect(mBoxesListAnimationDockWidget,
            &BoxesListAnimationDockWidget::viewedVerticalRangeChanged,
            mKeysView, &KeysView::setViewedVerticalRange);

    connect(sceneChooser, &SceneChooser::currentChanged,
            this, &BoxesListKeysViewWidget::setCurrentScene);
    sceneChooser->setCurrentScene(mDocument.fActiveScene);

    mBoxesListScrollArea->setFixedWidth(20*MIN_WIDGET_DIM);

    setLayout(mMainLayout);

    mFrameScrollBar = new FrameScrollBar(1, 1, MIN_WIDGET_DIM,
                                                   false, false, this);
//    connect(MemoryHandler::sGetInstance(), &MemoryHandler::memoryFreed,
//            frameScrollBar,
//            qOverload<>(&FrameScrollBar::update));
    connect(mFrameScrollBar, &FrameScrollBar::viewedFrameRangeChanged,
            this, [sceneChooser](const FrameRange& range){
        const auto scene = sceneChooser->getCurrentScene();
        if(scene) scene->anim_setAbsFrame(range.fMin);
    });
    mFrameScrollBar->setSizePolicy(QSizePolicy::MinimumExpanding,
                                  QSizePolicy::Maximum);
    mKeysViewLayout->insertWidget(0, mFrameScrollBar);

    mFrameRangeScrollBar = new FrameScrollBar(20, 200, MIN_WIDGET_DIM,
                                              true, true, this);

    connect(mFrameRangeScrollBar, &FrameScrollBar::viewedFrameRangeChanged,
            this, &BoxesListKeysViewWidget::setViewedFrameRange);
    mKeysViewLayout->addWidget(mFrameRangeScrollBar);
}

void BoxesListKeysViewWidget::setCurrentScene(Canvas * const scene) {
    if(mCurrentScene) {
        disconnect(mCurrentScene, nullptr, mFrameScrollBar, nullptr);
        disconnect(mCurrentScene, nullptr, this, nullptr);
    }

    mCurrentScene = scene;
    mKeysView->setCurrentScene(scene);
    if(scene) {
        connect(scene, &Canvas::currentFrameChanged,
                mFrameScrollBar, &FrameScrollBar::setFirstViewedFrame);
        connect(scene, &Canvas::newFrameRange,
                mFrameScrollBar, &FrameScrollBar::setCanvasFrameRange);

    }
}

void BoxesListKeysViewWidget::setGraphEnabled(const bool bT) {
    mKeysView->setGraphViewed(bT);
    mAnimationDockWidget->setVisible(bT);
}

void BoxesListKeysViewWidget::moveSlider(int val) {
    int diff = val%MIN_WIDGET_DIM;
    if(diff != 0) {
        val -= diff;
        mBoxesListScrollArea->verticalScrollBar()->setSliderPosition(val);
    }
    emit mBoxesListAnimationDockWidget->viewedVerticalRangeChanged(
                        val, val + mBoxesListScrollArea->height());
}

void BoxesListKeysViewWidget::connectToChangeWidthWidget(
                                    ChangeWidthWidget *changeWidthWidget) {
    connect(changeWidthWidget, &ChangeWidthWidget::widthSet,
            this, &BoxesListKeysViewWidget::setBoxesListWidth);
    setBoxesListWidth(changeWidthWidget->getCurrentWidth());
}

void BoxesListKeysViewWidget::setBoxesListWidth(const int width) {
    const int sizeHintWidth = mBoxesListMenuBar->sizeHint().width();
    const int cornerSizeHintWidth = mCornerMenuBar->sizeHint().width();
    const int widthT = width - mCornerMenuBar->sizeHint().width();
    if(widthT > sizeHintWidth + cornerSizeHintWidth) {
        mBoxesListMenuBar->setFixedWidth(sizeHintWidth);
        mMenuWidgetsCont->setFixedWidth(widthT - sizeHintWidth);
    } else {
        mMenuWidgetsCont->setFixedWidth(cornerSizeHintWidth);
        mBoxesListMenuBar->setFixedWidth(widthT - cornerSizeHintWidth);
    }
    mBoxesListScrollArea->setFixedWidth(width);
}

void BoxesListKeysViewWidget::addNewBelowThis() {
    mBoxesListAnimationDockWidget->addNewBoxesListKeysViewWidgetBelow(this);
}

void BoxesListKeysViewWidget::removeThis() {
    mBoxesListAnimationDockWidget->removeBoxesListKeysViewWidget(this);
}

void BoxesListKeysViewWidget::setBoxRule(const SWT_BoxRule rule) {
    mBoxesListWidget->getVisiblePartWidget()->setCurrentRule(rule);
    mMainWindow->queScheduledTasksAndUpdate();
}

void BoxesListKeysViewWidget::setRuleNone() {
    setBoxRule(SWT_BR_ALL);
}

void BoxesListKeysViewWidget::setRuleSelected() {
    setBoxRule(SWT_BR_SELECTED);
}

void BoxesListKeysViewWidget::setRuleAnimated() {
    setBoxRule(SWT_BR_ANIMATED);
}

void BoxesListKeysViewWidget::setRuleNotAnimated() {
    setBoxRule(SWT_BR_NOT_ANIMATED);
}

void BoxesListKeysViewWidget::setRuleVisible() {
    setBoxRule(SWT_BR_VISIBLE);
}

void BoxesListKeysViewWidget::setRuleHidden() {
    setBoxRule(SWT_BR_HIDDEN);
}

void BoxesListKeysViewWidget::setRuleUnloced() {
    setBoxRule(SWT_BR_UNLOCKED);
}

void BoxesListKeysViewWidget::setRuleLocked() {
    setBoxRule(SWT_BR_LOCKED);
}

void BoxesListKeysViewWidget::setTargetAll() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(&mDocument, SWT_TARGET_ALL);
    mMainWindow->queScheduledTasksAndUpdate();
}

void BoxesListKeysViewWidget::setTargetCurrentCanvas() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(
                mMainWindow->getCanvasWindow()->getCurrentCanvas(),
                SWT_TARGET_CURRENT_CANVAS);
    mMainWindow->queScheduledTasksAndUpdate();
}

void BoxesListKeysViewWidget::setTargetCurrentGroup() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(
                mMainWindow->getCanvasWindow()->getCurrentGroup(),
                SWT_TARGET_CURRENT_GROUP);
    mMainWindow->queScheduledTasksAndUpdate();
}

void BoxesListKeysViewWidget::setCurrentType(const SWT_Type type) {
    mBoxesListWidget->getVisiblePartWidget()->setCurrentType(type);
    mMainWindow->queScheduledTasksAndUpdate();
}

void BoxesListKeysViewWidget::setTypeAll() {
    setCurrentType(SWT_TYPE_ALL);
}

void BoxesListKeysViewWidget::setTypeSound() {
    setCurrentType(SWT_TYPE_SOUND);
}

void BoxesListKeysViewWidget::setTypeGraphics() {
    setCurrentType(SWT_TYPE_GRAPHICS);
}

void BoxesListKeysViewWidget::setSearchText(const QString &text) {
    mBoxesListWidget->getVisiblePartWidget()->setCurrentSearchText(text);
    mMainWindow->queScheduledTasksAndUpdate();
}

void BoxesListKeysViewWidget::setViewedFrameRange(
        const FrameRange& range) {
    mFrameRangeScrollBar->setViewedFrameRange(range);
    mFrameScrollBar->setDisplayedFrameRange(range);
    mKeysView->setFramesRange(range);
}

void BoxesListKeysViewWidget::setCanvasFrameRange(
        const FrameRange& range) {
    mFrameRangeScrollBar->setDisplayedFrameRange(range);
    setViewedFrameRange(mFrameRangeScrollBar->getViewedRange());
    mFrameRangeScrollBar->setCanvasFrameRange(range);
    mFrameScrollBar->setCanvasFrameRange(range);
}
