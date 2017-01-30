#include "qrealanimatorvalueslider.h"
#include "Animators/qrealanimator.h"
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
        QDoubleSlider::paint(p,
                       (mAnimator->isRecording() ? QColor(255, 200, 200) :
                                                   QColor(200, 200, 255)),
                       (mAnimator->isRecording() ? QColor(255, 160, 160) :
                                                   QColor(160, 160, 255)),
                       ((mAnimator->isRecording() &&
                        mAnimator->isKeyOnCurrentFrame()) ? Qt::red :
                                                            Qt::blue) );
    }
}

void QrealAnimatorValueSlider::setAnimator(QrealAnimator *animator) {
    if(mAnimator != NULL) {
        disconnect(mAnimator, SIGNAL(valueChangedSignal(qreal)),
                   this, SLOT(setValue(qreal)));
    }
    mAnimator = animator;
    if(mAnimator != NULL) {
        setNumberDecimals(mAnimator->getNumberDecimals());
        connect(mAnimator, SIGNAL(valueChangedSignal(qreal)),
                this, SLOT(setValue(qreal)));
        setValue(mAnimator->getCurrentValue());
    }
}
