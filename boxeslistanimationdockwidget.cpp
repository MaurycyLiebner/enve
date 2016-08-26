#include "boxeslistanimationdockwidget.h"
#include "mainwindow.h"

BoxesListAnimationDockWidget::BoxesListAnimationDockWidget(MainWindow *parent) :
    QWidget(parent)
{
    setMinimumSize(200, 200);
    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);
    mMainLayout->setSpacing(0);
    mMainLayout->setMargin(0);
    mFrameRangeScrollbar = new AnimatonWidgetScrollBar(20, 200,
                                                       20, 30,
                                                       10, true,
                                                       true, this);
    mAnimationWidgetScrollbar = new AnimatonWidgetScrollBar(1, 1,
                                                            10, 30,
                                                            1, false,
                                                            true, this);

    connect(mFrameRangeScrollbar, SIGNAL(viewedFramesChanged(int,int)),
            mAnimationWidgetScrollbar, SLOT(setMinMaxFrames(int,int)) );

    mAnimationWidgetScrollbar->setSizePolicy(QSizePolicy::MinimumExpanding,
                                             QSizePolicy::Maximum);
    mBoxesList = new BoxesList(parent, this);

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

    mRecordButton = new QPushButton(
                QIcon("pixmaps/icons/record_button.png"), "", this);
    mRecordButton->setSizePolicy(QSizePolicy::Maximum,
                               QSizePolicy::Maximum);

    mAddKeyButton = new QPushButton(
                QIcon("pixmaps/icons/add_key_button.png"), "", this);
    mAddKeyButton->setSizePolicy(QSizePolicy::Maximum,
                               QSizePolicy::Maximum);

    mRemoveKeyButton = new QPushButton(
                QIcon("pixmaps/icons/remove_key_button.png"), "", this);
    mRemoveKeyButton->setSizePolicy(QSizePolicy::Maximum,
                               QSizePolicy::Maximum);

    mControlButtonsLayout = new QHBoxLayout();
    mControlButtonsWidget = new QWidget(this);
    mControlButtonsWidget->setFixedWidth(201);
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
    mControlButtonsLayout->addWidget(mAddKeyButton);
    mControlButtonsLayout->addWidget(mRemoveKeyButton);

    mControlsLayout->addWidget(mControlButtonsWidget);
    mControlsLayout->addWidget(mAnimationWidgetScrollbar);

    mMainLayout->addLayout(mControlsLayout);
    mMainLayout->addWidget(mBoxesList);
    mMainLayout->addWidget(mFrameRangeScrollbar);
}

BoxesList *BoxesListAnimationDockWidget::getBoxesList()
{
    return mBoxesList;
}

