#include "boxeslistkeysviewwidget.h"
#include "animationwidgetscrollbar.h"
#include "mainwindow.h"
#include "boxeslistanimationdockwidget.h"
#include "BoxesList/boxsinglewidget.h"
#include "BoxesList/OptimalScrollArea/singlewidgetabstraction.h"
#include "BoxesList/boxscrollwidget.h"
#include "keysview.h"
#include "BoxesList/OptimalScrollArea/scrollarea.h"
#include "BoxesList/boxscrollwidgetvisiblepart.h"
#include "canvaswindow.h"
#include "animationdockwidget.h"
#include "global.h"
#include "canvas.h"

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
    mBoxesListMenuBar->addSeparator();
    QMenu *objectsMenu = mBoxesListMenuBar->addMenu("State");
    objectsMenu->addAction("All", this, SLOT(setRuleNone()));
    objectsMenu->addAction("Selected", this, SLOT(setRuleSelected()));
    objectsMenu->addAction("Animated", this, SLOT(setRuleAnimated()));
    objectsMenu->addAction("Not Animated", this, SLOT(setRuleNotAnimated()));
    objectsMenu->addAction("Visible", this, SLOT(setRuleVisible()));
    objectsMenu->addAction("Invisible", this, SLOT(setRuleInvisible()));
    objectsMenu->addAction("Unlocked", this, SLOT(setRuleUnloced()));
    objectsMenu->addAction("Locked", this, SLOT(setRuleLocked()));

    QMenu *targetMenu = mBoxesListMenuBar->addMenu("Target");
    targetMenu->addAction("All", this, SLOT(setTargetAll()));
    targetMenu->addAction("Current Canvas", this,
                          SLOT(setTargetCurrentCanvas()));
    targetMenu->addAction("Current Group", this,
                          SLOT(setTargetCurrentGroup()));
    QMenu *typeMenu = mBoxesListMenuBar->addMenu("Type");
    typeMenu->addAction("All", this, SLOT(setTypeAll()));
    typeMenu->addAction("Sound", this, SLOT(setTypeSound()));

    QMenu *viewMenu = mBoxesListMenuBar->addMenu("View");
    QAction *graphAct = viewMenu->addAction("Graph Editor");
    graphAct->setCheckable(true);
    connect(graphAct, SIGNAL(toggled(bool)),
            this, SLOT(setGraphEnabled(bool)));

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
    mSearchLine->setFixedHeight(FONT_HEIGHT);
    mSearchLine->setStyleSheet("background-color: rgb(255, 255, 255);"
                               "border-bottom: 0;"
                               "border-radius: 4px;"
                               "border: 1px solid rgb(100, 100, 100);"
                               "margin-top: 1px;"
                               "margin-bottom: 1px;");
    connect(mSearchLine, SIGNAL(textChanged(QString)),
            this, SLOT(setSearchText(QString)));
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
    mBoxesListScrollArea = new ScrollArea(this);
    mBoxesListWidget = new BoxScrollWidget(mBoxesListScrollArea);
    mBoxesListScrollArea->setWidget(mBoxesListWidget);
    mBoxesListLayout->addWidget(mBoxesListScrollArea);
    mMainLayout->addLayout(mBoxesListLayout);

    mKeysViewLayout = new QVBoxLayout();
    mKeysView = new KeysView(mBoxesListWidget->getVisiblePartWidget(),
                             this);
    mKeysViewLayout->addWidget(mKeysView);
    mAnimationDockWidget = new AnimationDockWidget(this, mKeysView);
    mKeysViewLayout->addWidget(mAnimationDockWidget);
    mAnimationDockWidget->hide();
    mMainLayout->addLayout(mKeysViewLayout);

    QHBoxLayout *keysViewScrollbarLayout = new QHBoxLayout();
    mKeysView->setLayout(keysViewScrollbarLayout);
    keysViewScrollbarLayout->setAlignment(Qt::AlignRight);
    keysViewScrollbarLayout->addWidget(
                mBoxesListScrollArea->verticalScrollBar());
    mBoxesListScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    keysViewScrollbarLayout->setContentsMargins(0, 0, 0, 0);

    connect(mBoxesListScrollArea->verticalScrollBar(),
            SIGNAL(valueChanged(int)),
            mBoxesListWidget, SLOT(changeVisibleTop(int)));
    connect(mBoxesListScrollArea, SIGNAL(heightChanged(int)),
            mBoxesListWidget, SLOT(changeVisibleHeight(int)));
    connect(mBoxesListScrollArea, SIGNAL(widthChanged(int)),
            mBoxesListWidget, SLOT(setWidth(int)));

    connect(mBoxesListScrollArea->verticalScrollBar(),
            SIGNAL(valueChanged(int)),
            this, SLOT(moveSlider(int)));
    connect(mKeysView, SIGNAL(wheelEventSignal(QWheelEvent*)),
            mBoxesListScrollArea, SLOT(callWheelEvent(QWheelEvent*)));

    connect(mBoxesListAnimationDockWidget, SIGNAL(visibleRangeChanged(int,int)),
            mKeysView, SLOT(setViewedRange(int,int)) );
    mBoxesListScrollArea->setFixedWidth(20*MIN_WIDGET_HEIGHT);

    setLayout(mMainLayout);

    mMainWindow->getCanvasWindow()->SWT_getAbstractionForWidget(
                mBoxesListWidget->getVisiblePartWidget());
    mBoxesListWidget->getVisiblePartWidget()->setCurrentRule(SWT_NoRule);
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(
                NULL,
                SWT_CurrentCanvas);

    setTopWidget(topWidget);
}

BoxesListKeysViewWidget::~BoxesListKeysViewWidget() {

}

void BoxesListKeysViewWidget::setGraphEnabled(const bool &bT) {
    mKeysView->setGraphViewed(bT);
    mAnimationDockWidget->setVisible(bT);
}

void BoxesListKeysViewWidget::setTopWidget(QWidget *topWidget) {
    if(mTopWidget != NULL) {
        delete mTopWidget;
    }
    if(topWidget == NULL) {
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
    emit mBoxesListAnimationDockWidget->visibleRangeChanged(
                        val,
                        val + mBoxesListScrollArea->height());
}

void BoxesListKeysViewWidget::connectToFrameWidget(
        AnimationWidgetScrollBar *frameRange) {
    connect(mKeysView, SIGNAL(changedViewedFrames(int,int)),
            frameRange, SIGNAL(viewedFramesChanged(int,int)));
    connect(frameRange, SIGNAL(viewedFramesChanged(int,int)),
            mKeysView, SLOT(setFramesRange(int,int)) );
    mKeysView->setFramesRange(frameRange->getFirstViewedFrame(),
                              frameRange->getLastViewedFrame());
}

void BoxesListKeysViewWidget::connectToChangeWidthWidget(
                                    ChangeWidthWidget *changeWidthWidget) {
    connect(changeWidthWidget, SIGNAL(widthSet(int)),
            this, SLOT(setBoxesListWidth(int)));
    setBoxesListWidth(changeWidthWidget->getCurrentWidth());
}

void BoxesListKeysViewWidget::setBoxesListWidth(const int &width) {
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
    mMainWindow->callUpdateSchedulers();
}

void BoxesListKeysViewWidget::setRuleSelected() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_Selected);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListKeysViewWidget::setRuleAnimated() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_Animated);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListKeysViewWidget::setRuleNotAnimated() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_NotAnimated);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListKeysViewWidget::setRuleVisible() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_Visible);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListKeysViewWidget::setRuleInvisible() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_Invisible);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListKeysViewWidget::setRuleUnloced() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_Unlocked);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListKeysViewWidget::setRuleLocked() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_Locked);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListKeysViewWidget::setTargetAll() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(
                mMainWindow->getCanvasWindow(),
                SWT_All);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListKeysViewWidget::setTargetCurrentCanvas() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(
                mMainWindow->getCanvasWindow()->getCurrentCanvas(),
                SWT_CurrentCanvas);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListKeysViewWidget::setTargetCurrentGroup() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(
                mMainWindow->getCanvasWindow()->getCurrentGroup(),
                SWT_CurrentGroup);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListKeysViewWidget::setTypeSound() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentType(&SingleWidgetTarget::SWT_isSingleSound);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListKeysViewWidget::setTypeAll() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentType(NULL);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListKeysViewWidget::setSearchText(const QString &text) {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentSearchText(text);
    mMainWindow->callUpdateSchedulers();
}

bool BoxesListKeysViewWidget::processFilteredKeyEvent(
        QKeyEvent *event) {
    return false;
    //return mKeysView->processFilteredKeyEvent(event);
}
