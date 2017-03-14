#include "boxeslistkeysviewwidget.h"
#include "animationwidgetscrollbar.h"
#include "mainwindow.h"
#include "boxeslistanimationdockwidget.h"
#include "BoxesList/boxsinglewidget.h"
#include "BoxesList/OptimalScrollArea/singlewidgetabstraction.h"

BoxesListKeysViewWidget::BoxesListKeysViewWidget(QWidget *topWidget,
                                                 BoxesListAnimationDockWidget *parent) :
    QWidget(parent) {
    if(topWidget == NULL) {
        mTopWidget = new QWidget(this);
        mTopWidget->setFixedHeight(20);
        mTopWidget->setStyleSheet("background-color: rgb(50, 50, 50)");
    } else {
        mTopWidget = topWidget;
    }
    mMainWindow = MainWindow::getInstance();
    mAnimationDockWidget = parent;

    mMainLayout = new QHBoxLayout(this);
    mMainLayout->setSpacing(0);
    mMainLayout->setMargin(0);

    mMenuLayout = new QHBoxLayout();
    mMenuLayout->setSpacing(0);
    mMenuLayout->setMargin(0);
    mBoxesListMenuBar = new QMenuBar(this);
    mBoxesListMenuBar->setFixedHeight(20);
    mBoxesListMenuBar->setStyleSheet("QMenuBar {"
                                        "border-top: 1px solid black;"
                                        "border-bottom: 1px solid black;"
                                     "}"
                                     "QMenuBar::item {"
                                        "margin-top: 2px;"
                                        "padding-top: 1px;"
                                     "}");
    QMenu *objectsMenu = mBoxesListMenuBar->addMenu("Objects");
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

    mSearchLine = new QLineEdit("", mBoxesListMenuBar);
    connect(mSearchLine, SIGNAL(textChanged(QString)),
            this, SLOT(setSearchText(QString)));
    mSearchLine->setSizePolicy(QSizePolicy::Maximum,
                               QSizePolicy::Fixed);
    mBoxesListMenuBar->setSizePolicy(QSizePolicy::Minimum,
                                     QSizePolicy::Fixed);

    mMenuLayout->addWidget(mBoxesListMenuBar);
    mMenuLayout->addWidget(mSearchLine);

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
    mKeysViewLayout->addWidget(mTopWidget);
    mKeysViewLayout->addWidget(mKeysView);
    mMainLayout->addLayout(mKeysViewLayout);

    QHBoxLayout *keysViewScrollbarLayout = new QHBoxLayout();
    mKeysView->setLayout(keysViewScrollbarLayout);
    keysViewScrollbarLayout->setAlignment(Qt::AlignRight);
    keysViewScrollbarLayout->addWidget(
                mBoxesListScrollArea->verticalScrollBar());
    mBoxesListScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    keysViewScrollbarLayout->setContentsMargins(0, 0, 0, 0);

    mBoxesListScrollArea->verticalScrollBar()->setSingleStep(
                BOX_HEIGHT);

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

    connect(mAnimationDockWidget, SIGNAL(visibleRangeChanged(int,int)),
            mKeysView, SLOT(setViewedRange(int,int)) );
    mBoxesListScrollArea->setFixedWidth(400);

    setLayout(mMainLayout);

    mMainWindow->getCanvasWidget()->SWT_getAbstractionForWidget(
                mBoxesListWidget->getVisiblePartWidget());
    mBoxesListWidget->getVisiblePartWidget()->setCurrentRule(SWT_NoRule);
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(
                NULL,
                SWT_CurrentCanvas);
}

BoxesListKeysViewWidget::~BoxesListKeysViewWidget() {
    delete mMainWindow->getCanvasWidget()->SWT_getAbstractionForWidget(
                mBoxesListWidget->getVisiblePartWidget());
}

void BoxesListKeysViewWidget::moveSlider(int val) {
    int diff = val%BOX_HEIGHT;
    if(diff != 0) {
        val -= diff;
        mBoxesListScrollArea->verticalScrollBar()->setSliderPosition(val);
    }
    emit mAnimationDockWidget->visibleRangeChanged(
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
}

void BoxesListKeysViewWidget::setBoxesListWidth(const int &width) {
    mBoxesListScrollArea->setFixedWidth(width);
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
                mMainWindow->getCanvasWidget(),
                SWT_All);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListKeysViewWidget::setTargetCurrentCanvas() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(
                mMainWindow->getCanvasWidget()->getCurrentCanvas(),
                SWT_CurrentCanvas);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListKeysViewWidget::setTargetCurrentGroup() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(
                mMainWindow->getCanvasWidget()->getCurrentGroup(),
                SWT_CurrentGroup);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListKeysViewWidget::setSearchText(const QString &text) {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentSearchText(text);
    mMainWindow->callUpdateSchedulers();
}

bool BoxesListKeysViewWidget::processFilteredKeyEvent(
        QKeyEvent *event) {
    return mKeysView->processFilteredKeyEvent(event);
}
