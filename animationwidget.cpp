#include "animationwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include "mainwindow.h"
#include "qrealpointvaluedialog.h"

AnimationWidget::AnimationWidget(QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    mAnimator = new QrealAnimator();

    mAnimator->setFrame(10);
    mAnimator->setCurrentValue(10.);
    mAnimator->saveCurrentValueAsKey();

    mAnimator->setFrame(30);
    mAnimator->setCurrentValue(5.);
    mAnimator->saveCurrentValueAsKey();

    mAnimator->setFrame(70);
    mAnimator->setCurrentValue(13.);
    mAnimator->saveCurrentValueAsKey();

    mAnimator->setFrame(110);
    mAnimator->setCurrentValue(2.);
    mAnimator->saveCurrentValueAsKey();
    setMinimumSize(200, 200);
}

void AnimationWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    mAnimator->draw(&p);
    p.end();
}

void AnimationWidget::setViewedFramesRange(int startFrame, int endFrame) {
    mAnimator->setViewedFramesRange(startFrame, endFrame);
    repaint();
}

void AnimationWidget::resizeEvent(QResizeEvent *)
{
    mAnimator->setRect(rect());
}

void AnimationWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton) {
        QrealPoint *point = mAnimator->getPointAt(event->pos());
        if(point == NULL) return;
        QrealPointValueDialog *dialog = new QrealPointValueDialog(point, this);
        dialog->show();
        connect(dialog, SIGNAL(repaintSignal()),
                this, SLOT(updateDrawnPath()) );
        connect(dialog, SIGNAL(finished(int)),
                this, SLOT(mergeKeysIfNeeded()) );
    } else if(event->button() == Qt::MiddleButton) {
        mAnimator->middlePress(event->pos());
    } else {
        mAnimator->mousePress(event->pos());
    }
    repaint();
}

void AnimationWidget::mouseMoveEvent(QMouseEvent *event) {
    if(event->buttons() == Qt::MiddleButton) {
        mAnimator->middleMove(event->pos());
        emit changedViewedFrames(mAnimator->getStartFrame(),
                                 mAnimator->getEndFrame());
    } else {
        mAnimator->mouseMove(event->pos());
    }
    repaint();
}

void AnimationWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::MiddleButton) {
        mAnimator->middleRelease();
    } else {
        mAnimator->mouseRelease();
    }
    repaint();
}

void AnimationWidget::wheelEvent(QWheelEvent *event)
{
    if(isCtrlPressed()) {
        if(event->delta() > 0) {
            mAnimator->incScale(0.1);
        } else {
            mAnimator->incScale(-0.1);
        }
    } else {
        if(event->delta() > 0) {
            mAnimator->incMinShownVal(0.1);
        } else {
            mAnimator->incMinShownVal(-0.1);
        }
    }
    repaint();
}

void AnimationWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Delete) {
        mAnimator->deletePressed();
    }
    repaint();
}

void AnimationWidget::updateDrawnPath()
{
    mAnimator->sortKeys();
    mAnimator->updateKeysPath();
    repaint();
}

void AnimationWidget::mergeKeysIfNeeded() {
    mAnimator->mergeKeysIfNeeded();
    repaint();
}
