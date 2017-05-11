#include "boxeslistanimationdockwidget.h"
#include "mainwindow.h"
#include <QKeyEvent>
#include "animationdockwidget.h"
#include <QScrollBar>
#include "BoxesList/boxscrollwidget.h"
#include "BoxesList/boxsinglewidget.h"
#include "verticalwidgetsstack.h"

ChangeWidthWidget::ChangeWidthWidget(QWidget *parent) :
    QWidget(parent) {
    setFixedWidth(10);
    setFixedHeight(4000);
    setCursor(Qt::SplitHCursor);
}

void ChangeWidthWidget::updatePos() {
    move(mCurrentWidth - 5, 0);
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
    int newWidth = mCurrentWidth + event->x() - mPressX;
    mCurrentWidth = qMax(200, newWidth);
    emit widthSet(mCurrentWidth);
    //mBoxesList->setFixedWidth(newWidth);
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

BoxesListAnimationDockWidget::BoxesListAnimationDockWidget(
        MainWindow *parent) :
    QWidget(parent) {
    mAddBoxesListKeysViewWidgetsBar->hide();
    mAddBoxesListKeysViewWidgetsBar->addAction(
                        "+", this,
                        SLOT(addNewBoxesListKeysViewWidget()));
    mAddBoxesListKeysViewWidgetsBar->setStyleSheet(
               "QMenuBar {"
                   "color: white;"
                   "background-color: rgb(50, 50, 50)"
               "} QMenuBar::item {"
                   "spacing: 3px;"
                   "padding: 1px 4px;"
                   "background: transparent;"
                   "border-radius: 4px;"
               "} QMenuBar::item:selected {"
                   "background: rgb(75, 75, 75);"
               "} QMenuBar::item:pressed {"
                   "background: rgb(90, 90, 90);"
               "}");
    mMainWindow = parent;
    setMinimumSize(200, 200);
    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);
    mMainLayout->setSpacing(0);
    mMainLayout->setMargin(0);

    mTimelineLayout = new QVBoxLayout();
    mTimelineLayout->setSpacing(0);
    mTimelineLayout->setMargin(0);

    mFrameRangeScrollbar = new AnimationWidgetScrollBar(20, 200,
                                                       20, 20,
                                                       true,
                                                       true, this);
    mAnimationWidgetScrollbar = new AnimationWidgetScrollBar(1, 1,
                                                            10, 20,
                                                            false,
                                                            false, this);
    mAnimationWidgetScrollbar->setTopBorderVisible(false);

    connect(mAnimationWidgetScrollbar, SIGNAL(viewedFramesChanged(int,int)),
            parent, SLOT(setCurrentFrame(int)) );

    connect(mFrameRangeScrollbar, SIGNAL(viewedFramesChanged(int,int)),
            mAnimationWidgetScrollbar, SLOT(setMinMaxFrames(int,int)) );


    mAnimationWidgetScrollbar->setSizePolicy(QSizePolicy::MinimumExpanding,
                                             QSizePolicy::Maximum);



//    mAnimationDockWidget = new AnimationDockWidget(mBoxesListWidget,
//                                                   mKeysView);
//    mKeysView->setAnimationDockWidget(mAnimationDockWidget);

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
    mResolutionComboBox->setEditable(true);
    mResolutionComboBox->lineEdit()->setInputMask("D00 %");
    mResolutionComboBox->setCurrentText("100 %");
    mResolutionComboBox->setInsertPolicy(QComboBox::NoInsert);
    mResolutionComboBox->setSizePolicy(QSizePolicy::Maximum,
                                       QSizePolicy::Maximum);
    connect(mResolutionComboBox, SIGNAL(currentTextChanged(QString)),
            this, SLOT(setResolutionFractionText(QString)));

    mPlayButton = new ActionButton(
                ":/icons/play.png",
                "start preview", this);
    mPlayButton->setCheckable(":/icons/stop.png");
    connect(mPlayButton, SIGNAL(toggled(bool)),
            this, SLOT(playStopPreview(bool)) );

    mAllPointsRecordButton = new ActionButton(
                ":/icons/recordSinglePoint.png",
                "start preview", this);
    mAllPointsRecordButton->setCheckable(":/icons/recordAllPoints.png");
    connect(mAllPointsRecordButton, SIGNAL(toggled(bool)),
            parent, SLOT(setAllPointsRecord(bool)) );

    mCtrlsAlwaysVisible = new ActionButton(
                ":/icons/ctrlsNotAlwaysVisible.png",
                "", this);

    mCtrlsAlwaysVisible->setCheckable(":/icons/ctrlsAlwaysVisible.png");
    connect(mCtrlsAlwaysVisible, SIGNAL(toggled(bool)),
            this, SLOT(setCtrlsAlwaysVisible(bool)) );

    mLocalPivot = new ActionButton(
                ":/icons/globalPivot.png",
                "", this);

    mLocalPivot->setCheckable(":/icons/localPivot.png");
    connect(mLocalPivot, SIGNAL(toggled(bool)),
            this, SLOT(setLocalPivot(bool)) );

    mToolBar = new QToolBar(this);
    mToolBar->setMovable(false);

    mToolBar->setIconSize(QSize(24, 24));
    mToolBar->addSeparator();

//    mControlButtonsLayout->addWidget(mGoToPreviousKeyButton);
//    mGoToPreviousKeyButton->setFocusPolicy(Qt::NoFocus);
//    mControlButtonsLayout->addWidget(mGoToNextKeyButton);
//    mGoToNextKeyButton->setFocusPolicy(Qt::NoFocus);
    mToolBar->addAction("Resolution:");
    mToolBar->addWidget(mResolutionComboBox);
    mToolBar->addSeparator();
    //mResolutionComboBox->setFocusPolicy(Qt::NoFocus);

    mToolBar->addWidget(mPlayButton);
    mPlayButton->setFocusPolicy(Qt::NoFocus);

    mToolBar->addSeparator();
    mToolBar->addWidget(mAllPointsRecordButton);
    mAllPointsRecordButton->setFocusPolicy(Qt::NoFocus);
    mToolBar->addWidget(mCtrlsAlwaysVisible);
    mCtrlsAlwaysVisible->setFocusPolicy(Qt::NoFocus);
    mToolBar->addWidget(mLocalPivot);
    mLocalPivot->setFocusPolicy(Qt::NoFocus);
    mToolBar->addSeparator();

    QWidget *spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    spacerWidget->setStyleSheet("QWidget { background-color: rgba(0, 0, 0, 0) }");
    mToolBar->addWidget(spacerWidget);

    mToolBar->addSeparator();

    mTimelineAction = mToolBar->addAction("Timeline",
                                       this, SLOT(setTimelineMode()));
    mTimelineAction->setCheckable(true);
    mTimelineAction->setChecked(true);
    mRenderAction = mToolBar->addAction("Render", this, SLOT(setRenderMode()));
    mRenderAction->setCheckable(true);

    mToolBar->addSeparator();

    mMainLayout->addWidget(mToolBar);

    mBoxesListKeysViewStack = new VerticalWidgetsStack(this);
    mTimelineLayout->addWidget(mBoxesListKeysViewStack);

    mTimelineLayout->addWidget(mAddBoxesListKeysViewWidgetsBar);

    mTimelineLayout->addWidget(mFrameRangeScrollbar, Qt::AlignBottom);

    mFrameRangeScrollbar->emitChange();

    mChww = new ChangeWidthWidget(mBoxesListKeysViewStack);

    mChww->updatePos();

    mFrameRangeScrollbar->raise();

    mTimelineWidget = new QWidget(this);
    mRenderWidget = new RenderWidget(this);
    mTimelineWidget->setLayout(mTimelineLayout);
    mMainLayout->addWidget(mTimelineWidget);
    mMainLayout->addWidget(mRenderWidget);
    mRenderWidget->hide();

    addNewBoxesListKeysViewWidget(0);
    //addNewBoxesListKeysViewWidget(1);
    //addNewBoxesListKeysViewWidget(0);
}

void BoxesListAnimationDockWidget::setResolutionFractionText(QString text) {
    text = text.remove(" %");
    mMainWindow->setResolutionFractionValue(text.toDouble()/100.);
}

void BoxesListAnimationDockWidget::addNewBoxesListKeysViewWidget(
                                        int id) {
    if(mBoxesListKeysViewStack->isHidden()) {
        mBoxesListKeysViewStack->show();
        mAddBoxesListKeysViewWidgetsBar->hide();
        setMinimumHeight(200);
        setMaximumHeight(2000);
    }
    if(id < 0) id = 0;
    id = qMin(id, mBoxesListKeysViewWidgets.count());
    BoxesListKeysViewWidget *newWidget;
    if(id == 0) {
        newWidget = new BoxesListKeysViewWidget(mAnimationWidgetScrollbar,
                                                this,
                                                mBoxesListKeysViewStack);
    } else {
        newWidget = new BoxesListKeysViewWidget(NULL,
                                                this,
                                                mBoxesListKeysViewStack);
    }
    newWidget->connectToChangeWidthWidget(mChww);
    newWidget->connectToFrameWidget(mFrameRangeScrollbar);
    mBoxesListKeysViewStack->insertWidget(id, newWidget);
    mBoxesListKeysViewWidgets.insert(id, newWidget);

    mChww->raise();
    mFrameRangeScrollbar->raise();
}

void BoxesListAnimationDockWidget::removeBoxesListKeysViewWidget(
                                        BoxesListKeysViewWidget *widget) {
    if(mBoxesListKeysViewWidgets.indexOf(widget) == 0) {
        if(mBoxesListKeysViewWidgets.count() > 1) {
            mBoxesListKeysViewWidgets.at(1)->setTopWidget(
                                            mAnimationWidgetScrollbar);
        } else {
            mMainLayout->insertWidget(1, mAnimationWidgetScrollbar);
            mAddBoxesListKeysViewWidgetsBar->show();
            mBoxesListKeysViewStack->hide();
            setMinimumHeight(80);
            setMaximumHeight(80);
        }
    }
    mBoxesListKeysViewWidgets.removeOne(widget);
    mBoxesListKeysViewStack->removeWidget(widget);
}

void BoxesListAnimationDockWidget::addNewBoxesListKeysViewWidgetBelow(
                                        BoxesListKeysViewWidget *widget) {
    addNewBoxesListKeysViewWidget(mBoxesListKeysViewStack->getIdOf(widget) + 1);
}

void BoxesListAnimationDockWidget::clearAll() {
    QList<BoxesListKeysViewWidget*> widgets = mBoxesListKeysViewWidgets;
    foreach(BoxesListKeysViewWidget *widget, widgets) {
        removeBoxesListKeysViewWidget(widget);
    }
}

RenderWidget *BoxesListAnimationDockWidget::getRenderWidget() {
    return mRenderWidget;
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
    foreach(BoxesListKeysViewWidget *blk, mBoxesListKeysViewWidgets) {
        blk->processFilteredKeyEvent(event);
    }
    if(processUnfilteredKeyEvent(event) ) return true;
    return false;//mBoxesList->processFilteredKeyEvent(event);
}

void BoxesListAnimationDockWidget::previewFinished() {
    //setPlaying(false);
    mPlayButton->setChecked(false);
}

void BoxesListAnimationDockWidget::playStopPreview(const bool &play) {
    if(play) {
        playPreview();
    } else {
        stopPreview();
    }
}

void BoxesListAnimationDockWidget::stopPreview() {
    mMainWindow->getCanvasWidget()->interruptPreview();
}


void BoxesListAnimationDockWidget::playPreview() {
    mMainWindow->getCanvasWidget()->playPreview();
}

void BoxesListAnimationDockWidget::setCtrlsAlwaysVisible(
        bool ctrlsAlwaysVisible) {
    BoxesGroup::setCtrlsAlwaysVisible(ctrlsAlwaysVisible);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListAnimationDockWidget::setLocalPivot(const bool &bT) {
    mMainWindow->getCanvasWidget()->setLocalPivot(bT);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListAnimationDockWidget::setTimelineMode() {
    mTimelineAction->setDisabled(true);
    mRenderAction->setDisabled(false);

    mRenderAction->setChecked(false);
    mTimelineWidget->show();
    mRenderWidget->hide();
}

void BoxesListAnimationDockWidget::setRenderMode() {
    mTimelineAction->setDisabled(false);
    mRenderAction->setDisabled(true);

    mTimelineAction->setChecked(false);
    mRenderWidget->show();
    mTimelineWidget->hide();
}

void BoxesListAnimationDockWidget::setCurrentFrame(int frame) {
    mAnimationWidgetScrollbar->setFirstViewedFrame(frame);
    mAnimationWidgetScrollbar->emitChange();
    mAnimationWidgetScrollbar->repaint();
}

void BoxesListAnimationDockWidget::updateSettingsForCurrentCanvas(
                                        Canvas *canvas) {
    disconnect(mResolutionComboBox, SIGNAL(currentTextChanged(QString)),
               this, SLOT(setResolutionFractionText(QString)));
    mResolutionComboBox->setCurrentText(
                QString::number(canvas->getResolutionFraction()*100.) + " %");
    connect(mResolutionComboBox, SIGNAL(currentTextChanged(QString)),
            this, SLOT(setResolutionFractionText(QString)));
}

void BoxesListAnimationDockWidget::setMinMaxFrame(
                                    const int &minFrame,
                                    const int &maxFrame) {
    mFrameRangeScrollbar->setMinMaxFrames(minFrame, maxFrame);
}
