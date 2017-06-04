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
    if(mAnimator != NULL) {
        mBlockAnimatorSignals = true;
        mAnimator->prp_startTransform();
    }
    QDoubleSlider::emitEditingStarted(value);
}

void QrealAnimatorValueSlider::emitValueChangedExternal(qreal value) {
    setDisplayedValue(value);
    emitValueChanged(value);
}

void QrealAnimatorValueSlider::emitValueChanged(qreal value)
{
    if(mAnimator != NULL) {
        mAnimator->qra_setCurrentValue(value);
    }
    QDoubleSlider::emitValueChanged(value);
}

void QrealAnimatorValueSlider::setValueExternal(const qreal &value) {
    if(mAnimator != NULL) {
        mBlockAnimatorSignals = true;
        mAnimator->qra_setCurrentValue(value);
        mBlockAnimatorSignals = false;
    }
    setDisplayedValue(value);
}

void QrealAnimatorValueSlider::emitEditingFinished(qreal value) {
    if(mAnimator != NULL) {
        mAnimator->prp_finishTransform();
        mBlockAnimatorSignals = false;
    }
    QDoubleSlider::emitEditingFinished(value);
}

void QrealAnimatorValueSlider::nullifyAnimator() {
    setAnimator(NULL);
}

void QrealAnimatorValueSlider::setValueFromAnimator(const qreal &val) {
    if(mBlockAnimatorSignals) return;
    setDisplayedValue(val);
    emit displayedValueChanged(val);
}

void QrealAnimatorValueSlider::paint(QPainter *p)
{
    if(mAnimator == NULL) {
        QDoubleSlider::paint(p);
    } else {
        QDoubleSlider::paint(p,
                       (mAnimator->prp_isRecording() ? QColor(255, 200, 200) :
                                                   QColor(255, 255, 255)),
                       (mAnimator->prp_isRecording() ? QColor(255, 160, 160) :
                                                   QColor(220, 220, 220)),
                       ((mAnimator->prp_isRecording() &&
                        mAnimator->prp_isKeyOnCurrentFrame()) ? Qt::red :
                                                            Qt::black));
    }
}

void QrealAnimatorValueSlider::setAnimator(QrealAnimator *animator) {
    if(mAnimator != NULL) {
        disconnect(mAnimator, SIGNAL(valueChangedSignal(qreal)),
                   this, SLOT(setValueFromAnimator(qreal)));
        disconnect(mAnimator, SIGNAL(beingDeleted()),
                   this, SLOT(nullifyAnimator()));
    }
    mAnimator = animator;
    if(mAnimator != NULL) {
        setNumberDecimals(mAnimator->getNumberDecimals());
        connect(mAnimator, SIGNAL(valueChangedSignal(qreal)),
                this, SLOT(setValueFromAnimator(qreal)));
        connect(mAnimator, SIGNAL(beingDeleted()),
                this, SLOT(nullifyAnimator()));

        setValueRange(mAnimator->getMinPossibleValue(),
                      mAnimator->getMaxPossibleValue());
        setPrefferedValueStep(mAnimator->getPrefferedValueStep());

        setDisplayedValue(mAnimator->qra_getCurrentValue());
    }
}

void QrealAnimatorValueSlider::openContextMenu(
        const QPoint &globalPos) {
    if(mAnimator == NULL) return;
    QMenu menu(this);

    if(mAnimator->prp_isKeyOnCurrentFrame()) {
        menu.addAction("Delete Keyframe",
                       mAnimator,
                       SLOT(anim_deleteCurrentKey()));
    } else {
        menu.addAction("Add Keyframe",
                       mAnimator,
                       SLOT(anim_saveCurrentValueAsKey()));
    }

    menu.addSeparator();

    QAction *recAct = menu.addAction("Recording");
    recAct->setCheckable(true);
    recAct->setChecked(mAnimator->prp_isRecording());
    connect(recAct, SIGNAL(toggled(bool)),
            mAnimator, SLOT(prp_setRecording(bool)));

    QAction *selectedAction = menu.exec(globalPos);
    if(selectedAction == NULL) {
        return;
    } else {
        MainWindow::getInstance()->callUpdateSchedulers();
    }
}
