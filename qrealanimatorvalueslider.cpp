#include "qrealanimatorvalueslider.h"
#include "qrealanimator.h"
#include "mainwindow.h"

QrealAnimatorValueSlider::QrealAnimatorValueSlider(qreal minVal, qreal maxVal,
                                                   QrealAnimator *animator,
                                                   QWidget *parent) :
    QDoubleSlider(minVal, maxVal, parent)
{
    mAnimator = animator;
    animator->addSlider(this);
}

QrealAnimatorValueSlider::~QrealAnimatorValueSlider()
{
    mAnimator->removeSlider(this);
}

void QrealAnimatorValueSlider::emitValueChanged(qreal value)
{
    mAnimator->setCurrentValue(value);
    MainWindow::getInstance()->callUpdateSchedulers();
}

void QrealAnimatorValueSlider::emitEditingFinished(qreal value)
{
    mAnimator->setCurrentValue(value, true);
}

void QrealAnimatorValueSlider::paint(QPainter *p)
{
    p->fillRect(rect(), (mAnimator->isRecording() ? QColor(255, 200, 200) : QColor(200, 200, 255)));
    if(!mTextEdit) {
        if(mShowValueSlider) {
            qreal valWidth = mValue*width()/(mMaxValue - mMinValue);
            p->fillRect(QRectF(0., 0., valWidth, height()),
                       (mAnimator->isRecording() ? QColor(255, 160, 160) : QColor(160, 160, 255)));
        }
        if(mShowName) {
            p->drawText(rect(), Qt::AlignCenter, mName + ": " + getValueString());
        } else {
            p->drawText(rect(), Qt::AlignCenter, getValueString());
        }
    }
    if(mAnimator->isKeyOnCurrentFrame()) {
        p->setPen(QPen(Qt::red));
    }
    p->drawRect(rect().adjusted(0, 0, -1, -1));
}
