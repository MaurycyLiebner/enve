#include "qrealanimatorvalueslider.h"
#include "qrealanimator.h"
#include "mainwindow.h"

QrealAnimatorValueSlider::QrealAnimatorValueSlider(qreal minVal, qreal maxVal,
                                                   qreal prefferedStep,
                                                   QWidget *parent) :
    QDoubleSlider(minVal, maxVal, prefferedStep, parent)
{

}

QrealAnimatorValueSlider::QrealAnimatorValueSlider(qreal minVal, qreal maxVal,
                                                   qreal prefferedStep,
                                                   QrealAnimator *animator,
                                                   QWidget *parent) :
    QDoubleSlider(minVal, maxVal, prefferedStep, parent)
{
    setAnimator(animator);
}

QrealAnimatorValueSlider::QrealAnimatorValueSlider(QrealAnimator *animator, QWidget *parent) :
    QDoubleSlider(animator->getMinPossibleValue(),
                  animator->getMaxPossibleValue(),
                  animator->getPrefferedValueStep(),
                  parent)
{
    setAnimator(animator);
}

QrealAnimatorValueSlider::~QrealAnimatorValueSlider()
{
}

void QrealAnimatorValueSlider::emitEditingStarted(qreal value)
{
    if(mAnimator == NULL) return;
    mAnimator->startTransform();
}

void QrealAnimatorValueSlider::emitValueChanged(qreal value)
{
    if(mAnimator == NULL) return;
    mAnimator->setCurrentValue(value);
    MainWindow::getInstance()->callUpdateSchedulers();
}

void QrealAnimatorValueSlider::emitEditingFinished(qreal value)
{
    if(mAnimator == NULL) return;
    mAnimator->finishTransform();
    MainWindow::getInstance()->callUpdateSchedulers();
}

void QrealAnimatorValueSlider::paint(QPainter *p)
{
    if(mAnimator == NULL) {
        QDoubleSlider::paint(p);
    } else {
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
        if(mAnimator->isRecording() && mAnimator->isKeyOnCurrentFrame()) {
            p->setPen(QPen(Qt::red));
        }
        p->drawRect(rect().adjusted(0, 0, -1, -1));
    }
}

void QrealAnimatorValueSlider::setAnimator(QrealAnimator *animator) {
    if(mAnimator != NULL) {
        disconnect(mAnimator, SIGNAL(valueChangedSignal(qreal)),
                   this, SLOT(setValue(qreal)));
    }
    mAnimator = animator;
    if(mAnimator != NULL) {
        connect(mAnimator, SIGNAL(valueChangedSignal(qreal)),
                this, SLOT(setValue(qreal)));
        setValue(mAnimator->getCurrentValue());
    }
}
