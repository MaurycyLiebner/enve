#include "animationdockwidget.h"

AnimationDockWidget::AnimationDockWidget(QWidget *parent) : QWidget(parent)
{
    mMainLayout = new QVBoxLayout(this);
    mMainLayout->setMargin(0);
    mMainLayout->setSpacing(0);
    mScrollBar = new AnimatonWidgetScrollBar(this);
    mAnimationWidget = new AnimationWidget(this);
    mMainLayout->addWidget(mScrollBar);
    mMainLayout->addWidget(mAnimationWidget);

    connect(mScrollBar, SIGNAL(viewedFramesChanged(int,int)),
            mAnimationWidget, SLOT(setViewedFramesRange(int,int)) );
    connect(mAnimationWidget, SIGNAL(changedViewedFrames(int,int)),
            mScrollBar, SLOT(setViewedFramesRange(int, int)) );

    mScrollBar->emitChange();

    setLayout(mMainLayout);
}
