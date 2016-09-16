#include "animationdockwidget.h"
#include <QStyleOption>

AnimationDockWidget::AnimationDockWidget(QWidget *parent) : QWidget(parent)
{
    mMainLayout = new QVBoxLayout(this);
    mMainLayout->setMargin(0);
    mMainLayout->setSpacing(0);

    mScrollBar = new AnimatonWidgetScrollBar(50, 200,
                                             10, 30,
                                             1, true,
                                             false, this);

    mAnimationWidget = new AnimationWidget(this);

    setStyleSheet("background-color: black");

    mButtonsLayout = new QHBoxLayout();

    mSymmetricButton = new QPushButton(
                QIcon("pixmaps/icons/ink_node_symmetric_white.png"),
                "", this);
//    mSymmetricButton->setSizePolicy(QSizePolicy::Maximum,
//                                    QSizePolicy::Maximum);
    connect(mSymmetricButton, SIGNAL(pressed()),
            mAnimationWidget, SLOT(setSymmetricCtrl()) );

    mSmoothButton = new QPushButton(
                QIcon("pixmaps/icons/ink_node_smooth_white.png"),
                "", this);
//    mSmoothButton->setSizePolicy(QSizePolicy::Maximum,
//                                 QSizePolicy::Maximum);
    connect(mSmoothButton, SIGNAL(pressed()),
            mAnimationWidget, SLOT(setSmoothCtrl()) );

    mCornerButton = new QPushButton(
                QIcon("pixmaps/icons/ink_node_cusp_white.png"),
                "", this);
//    mCornerButton->setSizePolicy(QSizePolicy::Maximum,
//                                 QSizePolicy::Maximum);
    connect(mCornerButton, SIGNAL(pressed()),
            mAnimationWidget, SLOT(setCornerCtrl()) );

    mTwoSideCtrlButton = new QPushButton(
                QIcon("pixmaps/icons/two_side_ctrl_white.png"),
                "", this);
//    mTwoSideCtrlButton->setSizePolicy(QSizePolicy::Maximum,
//                                     QSizePolicy::Maximum);
    connect(mTwoSideCtrlButton, SIGNAL(pressed()),
            mAnimationWidget, SLOT(setTwoSizeCtrl()) );

    mLeftSideCtrlButton = new QPushButton(
                QIcon("pixmaps/icons/left_side_ctrl_white.png"),
                "", this);
//    mLeftSideCtrlButton->setSizePolicy(QSizePolicy::Maximum,
//                                       QSizePolicy::Maximum);
    connect(mLeftSideCtrlButton, SIGNAL(pressed()),
            mAnimationWidget, SLOT(setLeftSideCtrl()) );

    mRightSideCtrlButton = new QPushButton(
                QIcon("pixmaps/icons/right_side_ctrl_white.png"),
                "", this);
//    mRightSideCtrlButton->setSizePolicy(QSizePolicy::Maximum,
//                                        QSizePolicy::Maximum);
    connect(mRightSideCtrlButton, SIGNAL(pressed()),
            mAnimationWidget, SLOT(setRightSideCtrl()) );

    mNoSideCtrlButton = new QPushButton(
                QIcon("pixmaps/icons/no_side_ctrl_white.png"),
                "", this);
//    mNoSideCtrlButton->setSizePolicy(QSizePolicy::Maximum,
//                                     QSizePolicy::Maximum);
    connect(mNoSideCtrlButton, SIGNAL(pressed()),
            mAnimationWidget, SLOT(setNoSideCtrl()) );

    mButtonsLayout->addWidget(mSymmetricButton);
    mButtonsLayout->addWidget(mSmoothButton);
    mButtonsLayout->addWidget(mCornerButton);
    mButtonsLayout->addWidget(mTwoSideCtrlButton);
    mButtonsLayout->addWidget(mLeftSideCtrlButton);
    mButtonsLayout->addWidget(mRightSideCtrlButton);
    mButtonsLayout->addWidget(mNoSideCtrlButton);

    mMainLayout->addWidget(mScrollBar);
    mMainLayout->addLayout(mButtonsLayout);
    mMainLayout->addWidget(mAnimationWidget);

    connect(mScrollBar, SIGNAL(viewedFramesChanged(int,int)),
            mAnimationWidget, SLOT(setViewedFramesRange(int,int)) );
    connect(mAnimationWidget, SIGNAL(changedViewedFrames(int,int)),
            mScrollBar, SLOT(setViewedFramesRange(int, int)) );

    mScrollBar->emitChange();

    setLayout(mMainLayout);
}

AnimationWidget *AnimationDockWidget::getAnimationWidget()
{
    return mAnimationWidget;
}

void AnimationDockWidget::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    p.end();
}
