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
#include "windowsinglewidgettarget.h"
#include <QToolButton>

BoxesListKeysViewWidget::BoxesListKeysViewWidget(
                            QWidget *topWidget,
                            BoxesListAnimationDockWidget *animationDock,
                            QWidget *parent) :
    QWidget(parent) {
    mMainWindow = MainWindow::getInstance();
    mBoxesListAnimationDockWidget = animationDock;

    mMainLayout = new QHBoxLayout(this);
    mMainLayout->setSpacing(0);
    mMainLayout->setMargin(0);

    mMenuLayout = new QHBoxLayout();
    mMenuLayout->setSpacing(0);
    mMenuLayout->setMargin(0);
    mBoxesListMenuBar = new QMenuBar(this);
    mBoxesListMenuBar->setFixedHeight(MIN_WIDGET_HEIGHT);
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
    QMenu * const objectsMenu = mBoxesListMenuBar->addMenu("State");
    objectsMenu->addAction("All", this, SLOT(setRuleNone()));
    objectsMenu->addAction("Selected", this, SLOT(setRuleSelected()));
    objectsMenu->addAction("Animated", this, SLOT(setRuleAnimated()));
    objectsMenu->addAction("Not Animated", this, SLOT(setRuleNotAnimated()));
    objectsMenu->addAction("Visible", this, SLOT(setRuleVisible()));
    objectsMenu->addAction("Invisible", this, SLOT(setRuleInvisible()));
    objectsMenu->addAction("Unlocked", this, SLOT(setRuleUnloced()));
    objectsMenu->addAction("Locked", this, SLOT(setRuleLocked()));

    QMenu * const targetMenu = mBoxesListMenuBar->addMenu("Target");
    targetMenu->addAction("All", this, SLOT(setTargetAll()));
    targetMenu->addAction("Current Canvas", this,
                          SLOT(setTargetCurrentCanvas()));
    targetMenu->addAction("Current Group", this,
                          SLOT(setTargetCurrentGroup()));
    QMenu * const typeMenu = mBoxesListMenuBar->addMenu("Type");
    typeMenu->addAction("All", this, SLOT(setTypeAll()));
    typeMenu->addAction("Sound", this, SLOT(setTypeSound()));

    //QMenu *viewMenu = mBoxesListMenuBar->addMenu("View");
    mGraphAct = mBoxesListMenuBar->addAction("Graph");
    mGraphAct->setCheckable(true);
    connect(mGraphAct, &QAction::toggled,
            this, &BoxesListKeysViewWidget::setGraphEnabled);

    mCornerMenuBar = new QMenuBar(this);
    mCornerMenuBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
    mCornerMenuBar->setFixedHeight(MIN_WIDGET_HEIGHT);
    mCornerMenuBar->setStyleSheet("QMenuBar {"
                                     "border-top: 0;"
                                     "border-bottom: 1px solid black;"
                                  "}");
    mCornerMenuBar->addSeparator();
    mCornerMenuBar->addAction(" + ", this, SLOT(addNewBelowThis()));
    mCornerMenuBar->addAction(" - ", this, SLOT(removeThis()));
    mCornerMenuBar->addSeparator();

    mMenuWidgetsLayout = new QHBoxLayout();
    mMenuWidgetsLayout->setAlignment(Qt::AlignRight);
    mMenuWidgetsLayout->setMargin(0);
    mMenuWidgetsLayout->setSpacing(0);
    mSearchLine = new QLineEdit("", mBoxesListMenuBar);
    mSearchLine->setFixedHeight(MIN_WIDGET_HEIGHT/*FONT_HEIGHT*/);
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

    const auto window = mMainWindow->getCanvasWindow()->getWindowSWT();
    mBoxesListWidget = new BoxScrollWidget(window, mBoxesListScrollArea);
    auto visiblePartWidget = mBoxesListWidget->getVisiblePartWidget();
    visiblePartWidget->setCurrentRule(SWT_NoRule);
    auto newTarget = window->getCanvasWindow()->getCurrentCanvas();
    visiblePartWidget->setCurrentTarget(newTarget, SWT_CurrentCanvas);

    mBoxesListScrollArea->setWidget(mBoxesListWidget);
    mBoxesListLayout->addWidget(mBoxesListScrollArea);
    mMainLayout->addLayout(mBoxesListLayout);

    mKeysViewLayout = new QVBoxLayout();
    mKeysView = new KeysView(mBoxesListWidget->getVisiblePartWidget(), this);
    connect(mKeysView, &KeysView::changedViewedFrames,
            this, &BoxesListKeysViewWidget::changedFrameRange);
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
    mBoxesListScrollArea->setFixedWidth(20*MIN_WIDGET_HEIGHT);

    setLayout(mMainLayout);

    setTopWidget(topWidget);
}

BoxesListKeysViewWidget::~BoxesListKeysViewWidget() {

}

void BoxesListKeysViewWidget::setGraphEnabled(const bool &bT) {
    mKeysView->setGraphViewed(bT);
    mAnimationDockWidget->setVisible(bT);
}

void BoxesListKeysViewWidget::setTopWidget(QWidget *topWidget) {
    if(mTopWidget) delete mTopWidget;
    if(!topWidget) {
        mTopWidget = new QWidget(this);
        mTopWidget->setFixedHeight(MIN_WIDGET_HEIGHT);
        mTopWidget->setStyleSheet("background-color: rgb(50, 50, 50)");
    } else {
        mTopWidget = topWidget;
    }
    mKeysViewLayout->insertWidget(0, mTopWidget);
}

void BoxesListKeysViewWidget::moveSlider(int val) {
    int diff = val%MIN_WIDGET_HEIGHT;
    if(diff != 0) {
        val -= diff;
        mBoxesListScrollArea->verticalScrollBar()->setSliderPosition(val);
    }
    emit mBoxesListAnimationDockWidget->viewedVerticalRangeChanged(
                        val,
                        val + mBoxesListScrollArea->height());
}

void BoxesListKeysViewWidget::connectToFrameWidget(
        AnimationWidgetScrollBar *frameRange) {
    mKeysView->setFramesRange(frameRange->getFirstViewedFrame(),
                              frameRange->getLastViewedFrame());
}

void BoxesListKeysViewWidget::connectToChangeWidthWidget(
                                    ChangeWidthWidget *changeWidthWidget) {
    connect(changeWidthWidget, &ChangeWidthWidget::widthSet,
            this, &BoxesListKeysViewWidget::setBoxesListWidth);
    setBoxesListWidth(changeWidthWidget->getCurrentWidth());
}

void BoxesListKeysViewWidget::setDisplayedFrameRange(const int &minFrame,
                                                  const int &maxFrame) {
    mKeysView->setFramesRange(minFrame, maxFrame);
}

void BoxesListKeysViewWidget::setBoxesListWidth(const int &width) {
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

void BoxesListKeysViewWidget::setRuleNone() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_NoRule);
    mMainWindow->queScheduledTasksAndUpdate();
}

void BoxesListKeysViewWidget::setRuleSelected() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_Selected);
    mMainWindow->queScheduledTasksAndUpdate();
}

void BoxesListKeysViewWidget::setRuleAnimated() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_Animated);
    mMainWindow->queScheduledTasksAndUpdate();
}

void BoxesListKeysViewWidget::setRuleNotAnimated() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_NotAnimated);
    mMainWindow->queScheduledTasksAndUpdate();
}

void BoxesListKeysViewWidget::setRuleVisible() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_Visible);
    mMainWindow->queScheduledTasksAndUpdate();
}

void BoxesListKeysViewWidget::setRuleInvisible() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_Invisible);
    mMainWindow->queScheduledTasksAndUpdate();
}

void BoxesListKeysViewWidget::setRuleUnloced() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_Unlocked);
    mMainWindow->queScheduledTasksAndUpdate();
}

void BoxesListKeysViewWidget::setRuleLocked() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_Locked);
    mMainWindow->queScheduledTasksAndUpdate();
}

void BoxesListKeysViewWidget::setTargetAll() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(
                mMainWindow->getCanvasWindow()->getWindowSWT(),
                SWT_All);
    mMainWindow->queScheduledTasksAndUpdate();
}

void BoxesListKeysViewWidget::setTargetCurrentCanvas() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(
                mMainWindow->getCanvasWindow()->getCurrentCanvas(),
                SWT_CurrentCanvas);
    mMainWindow->queScheduledTasksAndUpdate();
}

void BoxesListKeysViewWidget::setTargetCurrentGroup() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(
                mMainWindow->getCanvasWindow()->getCurrentGroup(),
                SWT_CurrentGroup);
    mMainWindow->queScheduledTasksAndUpdate();
}

void BoxesListKeysViewWidget::setTypeSound() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentType(&SingleWidgetTarget::SWT_isSingleSound);
    mMainWindow->queScheduledTasksAndUpdate();
}

void BoxesListKeysViewWidget::setTypeAll() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentType(nullptr);
    mMainWindow->queScheduledTasksAndUpdate();
}

void BoxesListKeysViewWidget::setSearchText(const QString &text) {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentSearchText(text);
    mMainWindow->queScheduledTasksAndUpdate();
}
