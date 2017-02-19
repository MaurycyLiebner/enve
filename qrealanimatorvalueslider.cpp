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

QrealAnimatorValueSlider::QrealAnimatorValueSlider(QrealAnimator *animator,
                                                   QWidget *parent) :
    QDoubleSlider(parent)
{
    setAnimator(animator);
}

QrealAnimatorValueSlider::QrealAnimatorValueSlider(QWidget *parent) :
    QrealAnimatorValueSlider(NULL, parent) {

}

QrealAnimatorValueSlider::QrealAnimatorValueSlider(QString name,
                                                   qreal minVal, qreal maxVal,
                                                   qreal prefferedStep,
                                                   QWidget *parent) :
    QDoubleSlider(name, minVal, maxVal, prefferedStep, parent)
{

}

void QrealAnimatorValueSlider::emitEditingStarted(qreal value)
{
    QDoubleSlider::emitEditingStarted(value);
    if(mAnimator == NULL) return;
    mAnimator->startTransform();
}

void QrealAnimatorValueSlider::emitValueChanged(qreal value)
{
    QDoubleSlider::emitValueChanged(value);
    if(mAnimator == NULL) return;
    mAnimator->setCurrentValue(value);
    MainWindow::getInstance()->callUpdateSchedulers();
}

void QrealAnimatorValueSlider::emitEditingFinished(qreal value)
{
    QDoubleSlider::emitEditingFinished(value);
    if(mAnimator == NULL) return;
    mAnimator->finishTransform();
    MainWindow::getInstance()->callUpdateSchedulers();
}

void QrealAnimatorValueSlider::nullifyAnimator() {
    setAnimator(NULL);
}

void QrealAnimatorValueSlider::paint(QPainter *p)
{
    if(mAnimator == NULL) {
        QDoubleSlider::paint(p);
    } else {
        QDoubleSlider::paint(p,
                       (mAnimator->isRecording() ? QColor(255, 200, 200) :
                                                   QColor(255, 255, 255)),
                       (mAnimator->isRecording() ? QColor(255, 160, 160) :
                                                   QColor(220, 220, 220)),
                       ((mAnimator->isRecording() &&
                        mAnimator->isKeyOnCurrentFrame()) ? Qt::red :
                                                            Qt::black));
    }
}

void QrealAnimatorValueSlider::setAnimator(QrealAnimator *animator) {
    if(mAnimator != NULL) {
        disconnect(mAnimator, SIGNAL(valueChangedSignal(qreal)),
                   this, SLOT(setValue(qreal)));
        disconnect(mAnimator, SIGNAL(beingDeleted()),
                   this, SLOT(nullifyAnimator()));
    }
    mAnimator = animator;
    if(mAnimator != NULL) {
        setNumberDecimals(mAnimator->getNumberDecimals());
        connect(mAnimator, SIGNAL(valueChangedSignal(qreal)),
                this, SLOT(setValue(qreal)));
        connect(mAnimator, SIGNAL(beingDeleted()),
                this, SLOT(nullifyAnimator()));

        setValueRange(mAnimator->getMinPossibleValue(),
                      mAnimator->getMaxPossibleValue());
        setPrefferedValueStep(mAnimator->getPrefferedValueStep());

        setValue(mAnimator->getCurrentValue());
    }
}

void QrealAnimatorValueSlider::openContextMenu(
        const QPoint &globalPos) {
    if(mAnimator == NULL) return;
    QMenu menu(this);

    if(mAnimator->isKeyOnCurrentFrame()) {
        menu.addAction("Delete Keyframe",
                       mAnimator,
                       SLOT(deleteCurrentKey()));
    } else {
        menu.addAction("Add Keyframe",
                       mAnimator,
                       SLOT(saveCurrentValueAsKey()));
    }

    menu.addSeparator();

    QAction *recAct = menu.addAction("Recording");
    recAct->setCheckable(true);
    recAct->setChecked(mAnimator->isRecording());
    connect(recAct, SIGNAL(toggled(bool)),
            mAnimator, SLOT(setRecording(bool)));

    QAction *selectedAction = menu.exec(globalPos);
    if(selectedAction == NULL) {
        return;
    } else {
        MainWindow::getInstance()->callUpdateSchedulers();
    }
}
