#include "complexanimatoritemwidget.h"
#include "Animators/qrealanimator.h"
#include "boxeslistwidget.h"
#include "mainwindow.h"
#include <QDrag>
#include <QMimeData>

ComplexAnimatorItemWidget::ComplexAnimatorItemWidget(QrealAnimator *target,
                                                     QWidget *parent) :
    QrealAnimatorItemWidget(target, parent) {

}

void ComplexAnimatorItemWidget::draw(QPainter *p) {
    int drawX = 0;
    QrealAnimatorItemWidget::draw(p);
    if(mTargetAnimator->isDescendantRecording() && !mTargetAnimator->isRecording()) {
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        p->setBrush(Qt::red);
        p->setPen(Qt::NoPen);
        p->drawEllipse(QPointF(BoxesListWidget::getListItemChildIndent()*0.5,
                              BoxesListWidget::getListItemHeight()*0.5),
                       2.5, 2.5);
        p->restore();
    }
    drawX += BoxesListWidget::getListItemChildIndent();
    if(mDetailsVisible) {
        p->drawPixmap(drawX, 0, *BoxesListWidget::ANIMATOR_CHILDREN_VISIBLE);
    } else {
        p->drawPixmap(drawX, 0, *BoxesListWidget::ANIMATOR_CHILDREN_HIDDEN);
    }
}

void ComplexAnimatorItemWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->x() < 20) {
        mTargetAnimator->setRecording(!mTargetAnimator->isRecording());
    }

    callUpdateSchedulers();
}

void ComplexAnimatorItemWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->x() < 20) {
    } else {
        setDetailsVisibile(!mDetailsVisible);
    }

    callUpdateSchedulers();
}

void ComplexAnimatorItemWidget::setDetailsVisibile(bool bT) {
    mDetailsVisible = bT;
    emit detailsVisibilityChanged(bT);
}
