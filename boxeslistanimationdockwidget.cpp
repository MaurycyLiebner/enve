#include "boxeslistanimationdockwidget.h"
#include "mainwindow.h"
#include <QKeyEvent>

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
    mBoxesList = new BoxesList(parent, this);
    connect(mFrameRangeScrollbar, SIGNAL(viewedFramesChanged(int,int)),
            mBoxesList, SLOT(setFramesRange(int,int)) );

    mControlsLayout = new QHBoxLayout();
    mControlsLayout->setAlignment(Qt::AlignLeft);

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

    mRecordButton = new QPushButton(
                QIcon("pixmaps/icons/not_recording.png"), "", this);
    mRecordButton->setSizePolicy(QSizePolicy::Maximum,
                               QSizePolicy::Maximum);
    mRecordButton->setCheckable(true);
    connect(mRecordButton, SIGNAL(toggled(bool)),
            this, SLOT(setRecording(bool)) );
    connect(mRecordButton, SIGNAL(toggled(bool)),
            parent, SLOT(setRecording(bool)) );

    mAllPointsRecordButton = new QPushButton(
                QIcon("pixmaps/icons/recordSinglePoint.png"), "", this);
    mAllPointsRecordButton->setSizePolicy(QSizePolicy::Maximum,
                                          QSizePolicy::Maximum);
    mAllPointsRecordButton->setCheckable(true);
    connect(mAllPointsRecordButton, SIGNAL(toggled(bool)),
            this, SLOT(setAllPointsRecord(bool)) );
    connect(mAllPointsRecordButton, SIGNAL(toggled(bool)),
            parent, SLOT(setAllPointsRecord(bool)) );

    mRemoveKeyButton = new QPushButton(
                QIcon("pixmaps/icons/remove_key_button.png"), "", this);
    mRemoveKeyButton->setSizePolicy(QSizePolicy::Maximum,
                               QSizePolicy::Maximum);

    mControlButtonsLayout = new QHBoxLayout();
    mControlButtonsWidget = new QWidget(this);
    mControlButtonsWidget->setFixedWidth(LIST_ITEM_MAX_WIDTH + 1);
    mControlButtonsWidget->setLayout(mControlButtonsLayout);
    mControlButtonsWidget->setStyleSheet("background-color: black");

    mControlButtonsLayout->setSpacing(0);
    mControlButtonsLayout->setMargin(0);
    mControlsLayout->setSpacing(0);
    mControlsLayout->setMargin(0);

    mControlButtonsLayout->addWidget(mGoToPreviousKeyButton);
    mControlButtonsLayout->addWidget(mGoToNextKeyButton);
    mControlButtonsLayout->addWidget(mPlayButton);

    mControlButtonsLayout->addWidget(mRecordButton);
    mControlButtonsLayout->addWidget(mAllPointsRecordButton);
    mControlButtonsLayout->addWidget(mRemoveKeyButton);

    mControlsLayout->addWidget(mControlButtonsWidget);
    mControlsLayout->addWidget(mAnimationWidgetScrollbar);

    mMainLayout->addLayout(mControlsLayout);
    mMainLayout->addWidget(mBoxesList);
    mMainLayout->addWidget(mFrameRangeScrollbar);

    mFrameRangeScrollbar->emitChange();
}

BoxesList *BoxesListAnimationDockWidget::getBoxesList()
{
    return mBoxesList;
}

bool BoxesListAnimationDockWidget::processUnfilteredKeyEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_Right && isCtrlPressed()) {
        setCurrentFrame(mMainWindow->getCurrentFrame() + 1);
    } else if(event->key() == Qt::Key_Left && isCtrlPressed()) {
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

void BoxesListAnimationDockWidget::setRecording(bool recording)
{
    if(recording) {
        mRecordButton->setIcon(QIcon("pixmaps/icons/recording.png") );
    } else {
        mRecordButton->setIcon(QIcon("pixmaps/icons/not_recording.png") );
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

void BoxesListAnimationDockWidget::setCurrentFrame(int frame) {
    mAnimationWidgetScrollbar->setFirstViewedFrame(frame);
    mAnimationWidgetScrollbar->emitChange();
    mAnimationWidgetScrollbar->repaint();
}
