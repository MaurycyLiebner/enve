#include "boxeslistanimationdockwidget.h"
#include "mainwindow.h"
#include <QKeyEvent>
#include "animationdockwidget.h"
#include <QScrollBar>
#include "BoxesList/boxscrollwidget.h"

ChangeWidthWidget::ChangeWidthWidget(QWidget *boxesList, QWidget *parent) :
    QWidget(parent) {
    mBoxesList = boxesList;
    setFixedWidth(10);
    setFixedHeight(4000);
    setCursor(Qt::SplitHCursor);
}

void ChangeWidthWidget::updatePos()
{
    move(mBoxesList->width() - 5, 0);
}

void ChangeWidthWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    if(mPressed) {
        p.fillRect(rect().adjusted(3, 0, -4, 0), Qt::black);
    } else if(mHover) {
        p.fillRect(rect().adjusted(4, 0, -4, 0), Qt::black);
    } else {
        p.fillRect(rect().adjusted(5, 0, -4, 0), Qt::black);
    }
    p.end();
}

void ChangeWidthWidget::mouseMoveEvent(QMouseEvent *event)
{
    int newWidth = mBoxesList->width() + event->x() - mPressX;
    newWidth = qMax(200, newWidth);
    BoxesListWidget::setListItemMaxWidth(newWidth);
    mBoxesList->setFixedWidth(newWidth);
    updatePos();
}

void ChangeWidthWidget::mousePressEvent(QMouseEvent *event)
{
    mPressed = true;
    mPressX = event->x();
    update();
}

void ChangeWidthWidget::mouseReleaseEvent(QMouseEvent *) {
    mPressed = false;
    update();
}

void ChangeWidthWidget::enterEvent(QEvent *) {
    mHover = true;
    update();
}

void ChangeWidthWidget::leaveEvent(QEvent *) {
    mHover = false;
    update();
}

void BoxesListAnimationDockWidget::moveSlider(int val) {
    int diff = val%BoxesListWidget::getListItemHeight();
    if(diff != 0) {
        val -= diff;
        mBoxesListScrollArea->verticalScrollBar()->setSliderPosition(val);
    }
    emit visibleRangeChanged(val, val + mBoxesListScrollArea->height());
}

BoxesListAnimationDockWidget::BoxesListAnimationDockWidget(
        MainWindow *parent) :
    QWidget(parent)
{
    mMainWindow = parent;
    setMinimumSize(200, 200);
    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);
    mMainLayout->setSpacing(0);
    mMainLayout->setMargin(0);
    mFrameRangeScrollbar = new AnimatonWidgetScrollBar(20, 200,
                                                       20, 20,
                                                       true,
                                                       true, this);
    mAnimationWidgetScrollbar = new AnimatonWidgetScrollBar(1, 1,
                                                            10, 20,
                                                            false,
                                                            false, this);

    connect(mAnimationWidgetScrollbar, SIGNAL(viewedFramesChanged(int,int)),
            parent, SLOT(setCurrentFrame(int)) );

    connect(mFrameRangeScrollbar, SIGNAL(viewedFramesChanged(int,int)),
            mAnimationWidgetScrollbar, SLOT(setMinMaxFrames(int,int)) );


    mAnimationWidgetScrollbar->setSizePolicy(QSizePolicy::MinimumExpanding,
                                             QSizePolicy::Maximum);

    mBoxesListScrollArea = new ScrollArea(this);
    mBoxesListWidget = new BoxScrollWidget(mBoxesListScrollArea);
    mBoxesListScrollArea->setWidget(mBoxesListWidget);
    mBoxesListWidget->setMainTarget(mMainWindow->getCanvasWidget());

    connect(mBoxesListScrollArea->verticalScrollBar(),
            SIGNAL(valueChanged(int)),
            mBoxesListWidget, SLOT(changeVisibleTop(int)));
    connect(mBoxesListScrollArea, SIGNAL(heightChanged(int)),
            mBoxesListWidget, SLOT(changeVisibleHeight(int)));
    connect(mBoxesListScrollArea, SIGNAL(widthChanged(int)),
            mBoxesListWidget, SLOT(setWidth(int)));

    mBoxesListScrollArea->verticalScrollBar()->setSingleStep(
                BoxesListWidget::getListItemHeight());
    connect(mBoxesListScrollArea->verticalScrollBar(),
            SIGNAL(valueChanged(int)),
            this, SLOT(moveSlider(int)));

//    Canvas *canvas = MainWindow::getInstance()->getCanvas();
//    connect(canvas, SIGNAL(changeChildZSignal(int,int)),
//            mBoxesList, SLOT(changeItemZ(int,int)));
//    connect(canvas, SIGNAL(addAnimatedBoundingBoxSignal(BoundingBox*)),
//            mBoxesList, SLOT(addItemForBox(BoundingBox*)));
//    connect(canvas, SIGNAL(removeAnimatedBoundingBoxSignal(BoundingBox*)),
//            mBoxesList, SLOT(removeItemForBox(BoundingBox*)));

    mBoxesListLayout = new QVBoxLayout();
    mBoxesListLayout->setSpacing(0);
    mBoxesListLayout->setMargin(0);

    mKeysView = new KeysView(mBoxesListWidget->getVisiblePartWidget(), this);
    connect(mKeysView, SIGNAL(changedViewedFrames(int,int)),
            mFrameRangeScrollbar, SLOT(setViewedFramesRange(int, int)) );
    connect(mKeysView, SIGNAL(changedViewedFrames(int,int)),
            mAnimationWidgetScrollbar, SLOT(setMinMaxFrames(int, int)) );
    connect(mFrameRangeScrollbar, SIGNAL(viewedFramesChanged(int,int)),
            mKeysView, SLOT(setFramesRange(int,int)) );

    connect(mKeysView, SIGNAL(wheelEventSignal(QWheelEvent*)),
            mBoxesListScrollArea, SLOT(callWheelEvent(QWheelEvent*)));

    connect(this, SIGNAL(visibleRangeChanged(int,int)),
            mKeysView, SLOT(setViewedRange(int,int)) );

    mAnimationDockWidget = new AnimationDockWidget(mBoxesListWidget,
                                                   mKeysView);
    mKeysView->setAnimationDockWidget(mAnimationDockWidget);

//    mGoToPreviousKeyButton = new QPushButton(
//                QIcon(":/icons/prev_key.png"), "", this);
//    mGoToPreviousKeyButton->setSizePolicy(QSizePolicy::Maximum,
//                                          QSizePolicy::Maximum);
//    mGoToNextKeyButton = new QPushButton(
//                QIcon(":/icons/next_key.png"), "", this);
//    mGoToNextKeyButton->setSizePolicy(QSizePolicy::Maximum,
//                                      QSizePolicy::Maximum);
    mResolutionComboBox = new QComboBox(this);
    mResolutionComboBox->addItem("100 %");
    mResolutionComboBox->addItem("75 %");
    mResolutionComboBox->addItem("50 %");
    mResolutionComboBox->addItem("25 %");
    mResolutionComboBox->setSizePolicy(QSizePolicy::Maximum,
                                       QSizePolicy::Maximum);
    connect(mResolutionComboBox, SIGNAL(currentIndexChanged(int)),
            mMainWindow, SLOT(setResolutionPercentId(int)));

    mPlayButton = new QPushButton(
                QIcon(":/icons/play.png"), "", this);
    mPlayButton->setSizePolicy(QSizePolicy::Maximum,
                               QSizePolicy::Maximum);
    connect(mPlayButton, SIGNAL(pressed()),
            this, SLOT(playPreview()) );

    mGraphEnabledButton = new QPushButton(
                QIcon(":/icons/graphDisabled.png"), "", this);
    mGraphEnabledButton->setSizePolicy(QSizePolicy::Maximum,
                               QSizePolicy::Maximum);
    mGraphEnabledButton->setCheckable(true);
    mGraphEnabledButton->setChecked(false);
    connect(mGraphEnabledButton, SIGNAL(toggled(bool)),
            this, SLOT(setGraphEnabled(bool)) );
    connect(mGraphEnabledButton, SIGNAL(toggled(bool)),
            parent, SLOT(setGraphEnabled(bool)) );

    mAllPointsRecordButton = new QPushButton(
                QIcon(":/icons/recordSinglePoint.png"), "", this);
    mAllPointsRecordButton->setSizePolicy(QSizePolicy::Maximum,
                                          QSizePolicy::Maximum);
    mAllPointsRecordButton->setCheckable(true);
    connect(mAllPointsRecordButton, SIGNAL(toggled(bool)),
            this, SLOT(setAllPointsRecord(bool)) );
    connect(mAllPointsRecordButton, SIGNAL(toggled(bool)),
            parent, SLOT(setAllPointsRecord(bool)) );

    mCtrlsAlwaysVisible = new QPushButton(
                QIcon(":/icons/ctrlsNotAlwaysVisible.png"), "", this);
    mCtrlsAlwaysVisible->setSizePolicy(QSizePolicy::Maximum,
                               QSizePolicy::Maximum);
    mCtrlsAlwaysVisible->setCheckable(true);
    mCtrlsAlwaysVisible->setChecked(false);
    connect(mCtrlsAlwaysVisible, SIGNAL(toggled(bool)),
            this, SLOT(setCtrlsAlwaysVisible(bool)) );

    mControlButtonsLayout = new QHBoxLayout();
    mControlButtonsLayout->setSpacing(0);
    mControlButtonsLayout->setMargin(0);

//    mControlButtonsLayout->addWidget(mGoToPreviousKeyButton);
//    mGoToPreviousKeyButton->setFocusPolicy(Qt::NoFocus);
//    mControlButtonsLayout->addWidget(mGoToNextKeyButton);
//    mGoToNextKeyButton->setFocusPolicy(Qt::NoFocus);
    mControlButtonsLayout->addWidget(mResolutionComboBox);
    mResolutionComboBox->setFocusPolicy(Qt::NoFocus);

    mControlButtonsLayout->addWidget(mPlayButton);
    mPlayButton->setFocusPolicy(Qt::NoFocus);

    mControlButtonsLayout->addWidget(mGraphEnabledButton);
    mGraphEnabledButton->setFocusPolicy(Qt::NoFocus);
    mControlButtonsLayout->addWidget(mAllPointsRecordButton);
    mAllPointsRecordButton->setFocusPolicy(Qt::NoFocus);
    mControlButtonsLayout->addWidget(mCtrlsAlwaysVisible);
    mCtrlsAlwaysVisible->setFocusPolicy(Qt::NoFocus);

    mBoxesListKeysViewLayout = new QHBoxLayout();
    mKeysViewLayout = new QVBoxLayout();

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

    mBoxesListMenuLayout = new QHBoxLayout();
    mSearchLine = new QLineEdit("", mBoxesListMenuBar);
    connect(mSearchLine, SIGNAL(textChanged(QString)),
            this, SLOT(setSearchText(QString)));
    mSearchLine->setSizePolicy(QSizePolicy::Maximum,
                               QSizePolicy::Fixed);

    mBoxesListMenuLayout->addWidget(mBoxesListMenuBar);
    mBoxesListMenuLayout->addWidget(mSearchLine);

    mBoxesListLayout->addLayout(mBoxesListMenuLayout);
    mBoxesListLayout->addWidget(mBoxesListScrollArea);

    mBoxesListKeysViewLayout->addLayout(mBoxesListLayout);
    mBoxesListKeysViewLayout->addLayout(mKeysViewLayout);
    QHBoxLayout *keysViewScrollbarLayout = new QHBoxLayout();
    mKeysView->setLayout(keysViewScrollbarLayout);
    keysViewScrollbarLayout->setAlignment(Qt::AlignRight);
    keysViewScrollbarLayout->addWidget(
                mBoxesListScrollArea->verticalScrollBar());
    mBoxesListScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    keysViewScrollbarLayout->setContentsMargins(0, 0, 0, 0);

    mKeysViewLayout->addWidget(mAnimationWidgetScrollbar);
    mKeysViewLayout->addWidget(mKeysView);
    mKeysViewLayout->addWidget(mAnimationDockWidget);
    mAnimationDockWidget->hide();


    mMainLayout->addLayout(mControlButtonsLayout);

    mMainLayout->addLayout(mBoxesListKeysViewLayout);

    mMainLayout->addWidget(mFrameRangeScrollbar);

    mFrameRangeScrollbar->emitChange();

    ChangeWidthWidget *chww = new ChangeWidthWidget(mBoxesListScrollArea,
                                                    this);
    mBoxesListScrollArea->setFixedWidth(400);
    mBoxesListMenuBar->setSizePolicy(QSizePolicy::Minimum,
                                     QSizePolicy::Fixed);
    chww->updatePos();

    mFrameRangeScrollbar->raise();
}

BoxScrollWidget *BoxesListAnimationDockWidget::getBoxesList()
{
    return mBoxesListWidget;
}

KeysView *BoxesListAnimationDockWidget::getKeysView()
{
    return mKeysView;
}

bool BoxesListAnimationDockWidget::processUnfilteredKeyEvent(
        QKeyEvent *event) {
    if(event->key() == Qt::Key_Right && mMainWindow->isCtrlPressed()) {
        setCurrentFrame(mMainWindow->getCurrentFrame() + 1);
    } else if(event->key() == Qt::Key_Left && mMainWindow->isCtrlPressed()) {
        setCurrentFrame(mMainWindow->getCurrentFrame() - 1);
    } else {
        return false;
    }
    return true;
}

bool BoxesListAnimationDockWidget::processFilteredKeyEvent(
        QKeyEvent *event) {
    if(processUnfilteredKeyEvent(event) ) return true;
    return false;//mBoxesList->processFilteredKeyEvent(event);
}

void BoxesListAnimationDockWidget::setPlaying(bool playing) {
    if(playing) {
        mPlayButton->setIcon(QIcon(":/icons/stop.png") );
        disconnect(mPlayButton, SIGNAL(pressed()),
                this, SLOT(playPreview()) );
        connect(mPlayButton, SIGNAL(pressed()),
                mMainWindow, SLOT(stopPreview()) );
    } else {
        mPlayButton->setIcon(QIcon(":/icons/play.png") );
        disconnect(mPlayButton, SIGNAL(pressed()),
                mMainWindow, SLOT(stopPreview()) );
        connect(mPlayButton, SIGNAL(pressed()),
                this, SLOT(playPreview()) );
    }
}

void BoxesListAnimationDockWidget::previewFinished()
{
    setPlaying(false);
}

void BoxesListAnimationDockWidget::playPreview()
{
    setPlaying(true);
    mMainWindow->playPreview();
}

void BoxesListAnimationDockWidget::setGraphEnabled(bool graphEnabled)
{
    if(graphEnabled) {
        mGraphEnabledButton->setIcon(QIcon(":/icons/graphEnabled.png") );
    } else {
        mGraphEnabledButton->setIcon(QIcon(":/icons/graphDisabled.png") );
    }
}

void BoxesListAnimationDockWidget::setAllPointsRecord(bool allPointsRecord)
{
    if(allPointsRecord) {
        mAllPointsRecordButton->setIcon(
                    QIcon(":/icons/recordAllPoints.png") );
    } else {
        mAllPointsRecordButton->setIcon(
                    QIcon(":/icons/recordSinglePoint.png") );
    }
}

void BoxesListAnimationDockWidget::setCtrlsAlwaysVisible(
        bool ctrlsAlwaysVisible) {
    if(ctrlsAlwaysVisible) {
        mCtrlsAlwaysVisible->setIcon(
                    QIcon(":/icons/ctrlsAlwaysVisible.png"));
    } else {
        mCtrlsAlwaysVisible->setIcon(
                    QIcon(":/icons/ctrlsNotAlwaysVisible.png"));
    }
    BoxesGroup::setCtrlsAlwaysVisible(ctrlsAlwaysVisible);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListAnimationDockWidget::setCurrentFrame(int frame) {
    mAnimationWidgetScrollbar->setFirstViewedFrame(frame);
    mAnimationWidgetScrollbar->emitChange();
    mAnimationWidgetScrollbar->repaint();
}

void BoxesListAnimationDockWidget::updateSettingsForCurrentCanvas(
                                        Canvas *canvas) {
    disconnect(mResolutionComboBox, SIGNAL(currentIndexChanged(int)),
               mMainWindow, SLOT(setResolutionPercentId(int)));
    mResolutionComboBox->setCurrentIndex(
                        qRound((1. - canvas->getResolutionPercent())*4.));
    connect(mResolutionComboBox, SIGNAL(currentIndexChanged(int)),
            mMainWindow, SLOT(setResolutionPercentId(int)));
}

void BoxesListAnimationDockWidget::setRuleNone() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_NoRule);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListAnimationDockWidget::setRuleSelected() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_Selected);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListAnimationDockWidget::setRuleAnimated() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_Animated);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListAnimationDockWidget::setRuleNotAnimated() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_NotAnimated);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListAnimationDockWidget::setRuleVisible() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_Visible);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListAnimationDockWidget::setRuleInvisible() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_Invisible);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListAnimationDockWidget::setRuleUnloced() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_Unlocked);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListAnimationDockWidget::setRuleLocked() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentRule(SWT_Locked);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListAnimationDockWidget::setTargetAll() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(
                mMainWindow->getCanvasWidget(),
                SWT_All);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListAnimationDockWidget::setTargetCurrentCanvas() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(
                mMainWindow->getCanvasWidget()->getCurrentCanvas(),
                SWT_CurrentCanvas);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListAnimationDockWidget::setTargetCurrentGroup() {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentTarget(
                mMainWindow->getCanvasWidget()->getCurrentGroup(),
                SWT_CurrentGroup);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListAnimationDockWidget::setSearchText(
        const QString &text) {
    mBoxesListWidget->getVisiblePartWidget()->
            setCurrentSearchText(text);
    mMainWindow->callUpdateSchedulers();
}
