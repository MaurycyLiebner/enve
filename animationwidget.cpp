#include "animationwidget.h"
#include <QPainter>
#include <QMouseEvent>

AnimationWidget::AnimationWidget(QWidget *parent) : QWidget(parent)
{
    mAnimator.setFrame(10);
    mAnimator.setCurrentValue(10.);
    mAnimator.saveCurrentValueAsKey();

    mAnimator.setFrame(30);
    mAnimator.setCurrentValue(5.);
    mAnimator.saveCurrentValueAsKey();

    mAnimator.setFrame(70);
    mAnimator.setCurrentValue(13.);
    mAnimator.saveCurrentValueAsKey();

    mAnimator.setFrame(110);
    mAnimator.setCurrentValue(2.);
    mAnimator.saveCurrentValueAsKey();
    setMinimumSize(200, 200);
}

void AnimationWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    mAnimator.draw(&p);
    p.end();
}

void AnimationWidget::setViewedFramesRange(int startFrame, int endFrame) {
    mAnimator.setViewedFramesRange(startFrame, endFrame);
    repaint();
}

void AnimationWidget::resizeEvent(QResizeEvent *)
{
    mAnimator.setRect(rect());
}

void AnimationWidget::mousePressEvent(QMouseEvent *event)
{
    mAnimator.mousePress(event->pos());
    repaint();
}

void AnimationWidget::mouseMoveEvent(QMouseEvent *event) {
    mAnimator.mouseMove(event->pos());
    repaint();
}

void AnimationWidget::mouseReleaseEvent(QMouseEvent *)
{
    mAnimator.mouseRelease();
    repaint();
}

void AnimationWidget::wheelEvent(QWheelEvent *event)
{
    if(event->delta() > 0) {
        mAnimator.incMargin(-5.);
    } else {
        mAnimator.incMargin(5.);
    }
    repaint();
}
