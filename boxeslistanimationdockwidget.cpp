#include "boxeslistanimationdockwidget.h"
#include "mainwindow.h"
#include <QKeyEvent>
#include "animationdockwidget.h"

BoxesListAnimationDockWidget::BoxesListAnimationDockWidget(MainWindow *parent) :
    QWidget(parent)
{
    mMainWindow = parent;
    setMinimumSize(200, 200);
    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);
    mMainLayout->setSpacing(0);
    mMainLayout->setMargin(0);
    mFrameRangeScrollbar = new AnimatonWidgetScrollBar(20, 200,
                                                       20, 30,
                                                       true,
                                                       true, this);
    mAnimationWidgetScrollbar = new AnimatonWidgetScrollBar(1, 1,
                                                            10, 30,
                                                            false,
                                                            true, this);

    connect(mAnimationWidgetScrollbar, SIGNAL(viewedFramesChanged(int,int)),
            parent, SLOT(setCurrentFrame(int)) );

    connect(mFrameRangeScrollbar, SIGNAL(viewedFramesChanged(int,int)),
            mAnimationWidgetScrollbar, SLOT(setMinMaxFrames(int,int)) );


    mAnimationWidgetScrollbar->setSizePolicy(QSizePolicy::MinimumExpanding,
                                             QSizePolicy::Maximum);
    mBoxesList = new BoxesList(this);

    mBoxesListLayout = new QVBoxLayout();
    mBoxesListLayout->setSpacing(0);
    mBoxesListLayout->setMargin(0);

    mKeysView = new KeysView(mBoxesList, this);
    connect(mKeysView, SIGNAL(changedViewedFrames(int,int)),
            mFrameRangeScrollbar, SLOT(setViewedFramesRange(int, int)) );
    connect(mKeysView, SIGNAL(changedViewedFrames(int,int)),
            mAnimationWidgetScrollbar, SLOT(setMinMaxFrames(int, int)) );
    connect(mFrameRangeScrollbar, SIGNAL(viewedFramesChanged(int,int)),
            mKeysView, SLOT(setFramesRange(int,int)) );

    mAnimationDockWidget = new AnimationDockWidget(mBoxesList, mKeysView);
    mKeysView->setAnimationDockWidget(mAnimationDockWidget);

    mGoToPreviousKeyButton = new QPushButton(
                QIcon("pixmaps/icons/prev_key_button.png"), "", this);
    mGoToPreviousKeyButton->setSizePolicy(QSizePolicy::Maximum,
                                          QSizePolicy::Maximum);
    mGoToNextKeyButton = new QPushButton(
                QIcon("pixmaps/icons/next_key_button.png"), "", this);
    mGoToNextKeyButton->setSizePolicy(QSizePolicy::Maximum,
                                      QSizePolicy::Maximum);
    mPlayButton = new QPushButton(
                QIcon("pixmaps/icons/play_button.png"), "", this);
    mPlayButton->setSizePolicy(QSizePolicy::Maximum,
                               QSizePolicy::Maximum);
    connect(mPlayButton, SIGNAL(pressed()),
            this, SLOT(playPreview()) );

    mGraphEnabledButton = new QPushButton(
                QIcon("pixmaps/icons/graphDisabled.png"), "", this);
    mGraphEnabledButton->setSizePolicy(QSizePolicy::Maximum,
                               QSizePolicy::Maximum);
    mGraphEnabledButton->setCheckable(true);
    mGraphEnabledButton->setChecked(false);
    connect(mGraphEnabledButton, SIGNAL(toggled(bool)),
            this, SLOT(setGraphEnabled(bool)) );
    connect(mGraphEnabledButton, SIGNAL(toggled(bool)),
            parent, SLOT(setGraphEnabled(bool)) );

    mAllPointsRecordButton = new QPushButton(
                QIcon("pixmaps/icons/recordSinglePoint.png"), "", this);
    mAllPointsRecordButton->setSizePolicy(QSizePolicy::Maximum,
                                          QSizePolicy::Maximum);
    mAllPointsRecordButton->setCheckable(true);
    connect(mAllPointsRecordButton, SIGNAL(toggled(bool)),
            this, SLOT(setAllPointsRecord(bool)) );
    connect(mAllPointsRecordButton, SIGNAL(toggled(bool)),
            parent, SLOT(setAllPointsRecord(bool)) );

    mCtrlsAlwaysVisible = new QPushButton(
                QIcon("pixmaps/icons/ctrlsNotAlwaysVisible.png"), "", this);
    mCtrlsAlwaysVisible->setSizePolicy(QSizePolicy::Maximum,
                               QSizePolicy::Maximum);
    mCtrlsAlwaysVisible->setCheckable(true);
    mCtrlsAlwaysVisible->setChecked(false);
    connect(mCtrlsAlwaysVisible, SIGNAL(toggled(bool)),
            this, SLOT(setCtrlsAlwaysVisible(bool)) );

    mControlButtonsLayout = new QHBoxLayout();
    mControlButtonsWidget = new QWidget(this);
    mControlButtonsWidget->setFixedHeight(30);
    mControlButtonsWidget->setLayout(mControlButtonsLayout);
    mControlButtonsWidget->setStyleSheet("QWidget {"
                                            "background-color: rgb(0, 0, 0);"
                                         "}"
                                         "QPushButton {"
                                            "qproperty-iconSize: 20px;"
                                            "border: 1px solid black;"
                                            "background-color: rgb(55, 55, 55);"
                                         "}");

    mControlButtonsLayout->setSpacing(0);
    mControlButtonsLayout->setMargin(0);

    mControlButtonsLayout->addWidget(mGoToPreviousKeyButton);
    mGoToPreviousKeyButton->setFocusPolicy(Qt::NoFocus);
    mControlButtonsLayout->addWidget(mGoToNextKeyButton);
    mGoToNextKeyButton->setFocusPolicy(Qt::NoFocus);
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

    mBoxesListLayout->addWidget(mControlButtonsWidget);
    mBoxesListLayout->addWidget(mBoxesList);

    mBoxesListKeysViewLayout->addLayout(mBoxesListLayout);
    mBoxesListKeysViewLayout->addLayout(mKeysViewLayout);

    mKeysViewLayout->addWidget(mAnimationWidgetScrollbar);
    mKeysViewLayout->addWidget(mKeysView);
    mKeysViewLayout->addWidget(mAnimationDockWidget);
    mAnimationDockWidget->hide();

    mMainLayout->addLayout(mBoxesListKeysViewLayout);


    mMainLayout->addWidget(mFrameRangeScrollbar);

    mFrameRangeScrollbar->emitChange();

    ChangeWidthWidget *chww = new ChangeWidthWidget(mBoxesList, this);
    chww->updatePos();

    mFrameRangeScrollbar->raise();
}

BoxesList *BoxesListAnimationDockWidget::getBoxesList()
{
    return mBoxesList;
}

KeysView *BoxesListAnimationDockWidget::getKeysView()
{
    return mKeysView;
}

bool BoxesListAnimationDockWidget::processUnfilteredKeyEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_Right && mMainWindow->isCtrlPressed()) {
        setCurrentFrame(mMainWindow->getCurrentFrame() + 1);
    } else if(event->key() == Qt::Key_Left && mMainWindow->isCtrlPressed()) {
        setCurrentFrame(mMainWindow->getCurrentFrame() - 1);
    } else {
        return false;
    }
    return true;
}

bool BoxesListAnimationDockWidget::processFilteredKeyEvent(QKeyEvent *event) {
    if(processUnfilteredKeyEvent(event) ) return true;
    return mBoxesList->processFilteredKeyEvent(event);
}

void BoxesListAnimationDockWidget::setPlaying(bool playing) {
    if(playing) {
        mPlayButton->setIcon(QIcon("pixmaps/icons/stop_button.png") );
        disconnect(mPlayButton, SIGNAL(pressed()),
                this, SLOT(playPreview()) );
        connect(mPlayButton, SIGNAL(pressed()),
                mMainWindow, SLOT(stopPreview()) );
    } else {
        mPlayButton->setIcon(QIcon("pixmaps/icons/play_button.png") );
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
        mGraphEnabledButton->setIcon(QIcon("pixmaps/icons/graphEnabled.png") );
    } else {
        mGraphEnabledButton->setIcon(QIcon("pixmaps/icons/graphDisabled.png") );
    }
}

void BoxesListAnimationDockWidget::setAllPointsRecord(bool allPointsRecord)
{
    if(allPointsRecord) {
        mAllPointsRecordButton->setIcon(
                    QIcon("pixmaps/icons/recordAllPoints.png") );
    } else {
        mAllPointsRecordButton->setIcon(
                    QIcon("pixmaps/icons/recordSinglePoint.png") );
    }
}

void BoxesListAnimationDockWidget::setCtrlsAlwaysVisible(bool ctrlsAlwaysVisible) {
    if(ctrlsAlwaysVisible) {
        mCtrlsAlwaysVisible->setIcon(
                    QIcon("pixmaps/icons/ctrlsAlwaysVisible.png"));
    } else {
        mCtrlsAlwaysVisible->setIcon(
                    QIcon("pixmaps/icons/ctrlsNotAlwaysVisible.png"));
    }
    BoxesGroup::setCtrlsAlwaysVisible(ctrlsAlwaysVisible);
    mMainWindow->callUpdateSchedulers();
}

void BoxesListAnimationDockWidget::setCurrentFrame(int frame) {
    mAnimationWidgetScrollbar->setFirstViewedFrame(frame);
    mAnimationWidgetScrollbar->emitChange();
    mAnimationWidgetScrollbar->repaint();
}
