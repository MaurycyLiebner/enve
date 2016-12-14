#include "qrealanimatoritemwidget.h"
#include "boxeslistwidget.h"
#include "mainwindow.h"
#include "qrealanimatorvalueslider.h"

QrealAnimatorItemWidget::QrealAnimatorItemWidget(QrealAnimator *target,
                                                 QWidget *parent) :
    QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);
    setFixedHeight(BoxesListWidget::getListItemHeight());
    mTargetAnimator = target;
    if(!target->isComplexAnimator()) {
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 20, 0);
        layout->setAlignment(Qt::AlignRight);
        setLayout(layout);
        QrealAnimatorValueSlider *valueSlider = new QrealAnimatorValueSlider(target);
        valueSlider->setWheelInteractionEnabled(false);
        layout->addWidget(valueSlider);
    }
}

void QrealAnimatorItemWidget::draw(QPainter *p) {
    if(mTargetAnimator->isCurrentAnimator()) {
        p->fillRect(-5, 0,
                   5, height(),
                   mTargetAnimator->getAnimatorColor());
        p->fillRect(0, 0,
                   BoxesListWidget::getListItemMaxWidth(), BoxesListWidget::getListItemHeight(),
                   QColor(255, 255, 255, 125));
    }
    int drawX = 0;
    if(mTargetAnimator->isRecording()) {
        p->drawPixmap(drawX, 0, *BoxesListWidget::ANIMATOR_RECORDING);
    } else {
        p->drawPixmap(drawX, 0, *BoxesListWidget::ANIMATOR_NOT_RECORDING);
    }
    p->setPen(Qt::black);
    drawX += 2*BoxesListWidget::getListItemChildIndent();
    p->drawText(drawX, 0,
               width() - 80.,
               BoxesListWidget::getListItemHeight(),
               Qt::AlignVCenter | Qt::AlignLeft,
               mTargetAnimator->getName() );
//    p->setPen(Qt::blue);
//    p->drawText(width() - 80., 0,
//               70., BoxesListWidget::getListItemHeight(),
//               Qt::AlignVCenter | Qt::AlignLeft,
//               " " + mTargetAnimator->getValueText() );
}

void QrealAnimatorItemWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    draw(&p);
    p.end();
}

void QrealAnimatorItemWidget::mousePressEvent(QMouseEvent *event) {
    if(event->button() == Qt::RightButton) {
        mTargetAnimator->openContextMenu(event->globalPos());
    } else {
        if(event->x() < 20) {
            mTargetAnimator->setRecording(!mTargetAnimator->isRecording());
        }
    }

    MainWindow::getInstance()->callUpdateSchedulers();
}

void QrealAnimatorItemWidget::drawKeys(QPainter *p, qreal pixelsPerFrame,
                                       int animatorTop, int maxY,
                                       int minViewedFrame, int maxViewedFrame) {
    mTargetAnimator->drawKeys(p, pixelsPerFrame, animatorTop,
                              minViewedFrame, maxViewedFrame);
}
