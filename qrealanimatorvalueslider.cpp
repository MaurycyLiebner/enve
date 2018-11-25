#include "qrealanimatorvalueslider.h"
#include "Animators/qrealanimator.h"
#include "mainwindow.h"
#include "Properties/intproperty.h"

QrealAnimatorValueSlider::QrealAnimatorValueSlider(qreal minVal, qreal maxVal,
                                                   qreal prefferedStep,
                                                   QWidget *parent) :
    QDoubleSlider(minVal, maxVal, prefferedStep, parent) {

}

QrealAnimatorValueSlider::QrealAnimatorValueSlider(qreal minVal, qreal maxVal,
                                                   qreal prefferedStep,
                                                   QrealAnimator *animator,
                                                   QWidget *parent) :
    QDoubleSlider(minVal, maxVal, prefferedStep, parent) {
    setAnimator(animator);
}

QrealAnimatorValueSlider::QrealAnimatorValueSlider(QrealAnimator *animator,
                                                   QWidget *parent) :
    QDoubleSlider(parent) {
    setAnimator(animator);
}

QrealAnimatorValueSlider::QrealAnimatorValueSlider(QWidget *parent) :
    QrealAnimatorValueSlider(nullptr, parent) {

}

QrealAnimatorValueSlider::QrealAnimatorValueSlider(QString name,
                                                   qreal minVal, qreal maxVal,
                                                   qreal prefferedStep,
                                                   QWidget *parent) :
    QDoubleSlider(name, minVal, maxVal, prefferedStep, parent) {

}

void QrealAnimatorValueSlider::emitEditingStarted(qreal value) {
    if(mAnimator != nullptr) {
        mBlockAnimatorSignals = true;
        mAnimator->prp_startTransform();
    }
    QDoubleSlider::emitEditingStarted(value);
}

void QrealAnimatorValueSlider::emitValueChangedExternal(qreal value) {
    setDisplayedValue(value);
    emitValueChanged(value);
}

void QrealAnimatorValueSlider::emitValueChanged(qreal value) {
    if(mAnimator != nullptr) {
        if(mAnimator->SWT_isQrealAnimator()) {
            SPtrGetAs(mAnimator, QrealAnimator)->qra_setCurrentValue(value);
        } else if(mAnimator->SWT_isIntProperty()) {
            SPtrGetAs(mAnimator, IntProperty)->setCurrentValue(qRound(value));
        }
    }
    QDoubleSlider::emitValueChanged(value);
}

void QrealAnimatorValueSlider::setValueExternal(qreal value) {
    if(mAnimator != nullptr) {
        mBlockAnimatorSignals = true;
        if(mAnimator->SWT_isQrealAnimator()) {
            SPtrGetAs(mAnimator, QrealAnimator)->qra_setCurrentValue(value);
        } else if(mAnimator->SWT_isIntProperty()) {
            SPtrGetAs(mAnimator, IntProperty)->setCurrentValue(qRound(value));
        }
        mBlockAnimatorSignals = false;
    }
    setDisplayedValue(value);
}

void QrealAnimatorValueSlider::emitEditingFinished(qreal value) {
    if(mAnimator != nullptr) {
        mAnimator->prp_finishTransform();
        mBlockAnimatorSignals = false;
    }
    QDoubleSlider::emitEditingFinished(value);
}

void QrealAnimatorValueSlider::nullifyAnimator() {
    clearAnimator();
}

void QrealAnimatorValueSlider::setValueFromAnimator(qreal val) {
    if(mBlockAnimatorSignals) return;
    setDisplayedValue(val);
    emit displayedValueChanged(val);
}

void QrealAnimatorValueSlider::paint(QPainter *p) {
    if(mAnimator == nullptr) {
        QDoubleSlider::paint(p);
    } else {
        if(isTargetDisabled()) {
            p->setOpacity(.5);
        }
        QDoubleSlider::paint(p,
                       (mAnimator->prp_isRecording() ? QColor(255, 200, 200) :
                                                   QColor(255, 255, 255)),
                       (mAnimator->prp_isRecording() ? QColor(255, 160, 160) :
                                                   QColor(220, 220, 220)),
                       ((mAnimator->prp_isRecording() &&
                        mAnimator->prp_isKeyOnCurrentFrame()) ? Qt::red :
                                                            Qt::black));
        p->setOpacity(1.);
    }
}

void QrealAnimatorValueSlider::clearAnimator() {
    if(mAnimator != nullptr) {
        disconnect(mAnimator.get(), nullptr, this, nullptr);
    }
    mAnimator = nullptr;
}

void QrealAnimatorValueSlider::setAnimator(QrealAnimator *animator) {
    if(animator == mAnimator) return;
    clearAnimator();
    mAnimator = animator;
    if(mAnimator != nullptr) {
        setNumberDecimals(animator->getNumberDecimals());
        connect(animator.get(), SIGNAL(valueChangedSignal(qreal)),
                this, SLOT(setValueFromAnimator(qreal)));
        connect(animator.get(), SIGNAL(beingDeleted()),
                this, SLOT(nullifyAnimator()));

        setValueRange(animator->getMinPossibleValue(),
                      animator->getMaxPossibleValue());
        setPrefferedValueStep(animator->getPrefferedValueStep());

        setDisplayedValue(animator->qra_getCurrentValue());
    }
}

void QrealAnimatorValueSlider::setIntAnimator(IntProperty *animator) {
    if(animator == mAnimator) return;
    clearAnimator();
    if(animator != nullptr) {
        setNumberDecimals(0);
        connect(animator, SIGNAL(valueChangedSignal(qreal)),
                this, SLOT(setValueFromAnimator(qreal)));
        connect(animator, SIGNAL(beingDeleted()),
                this, SLOT(nullifyAnimator()));

        setValueRange(animator->getMinValue(),
                      animator->getMaxValue());
        setPrefferedValueStep(1);

        setDisplayedValue(animator->getValue());
        mAnimator = animator;
    }
}

bool QrealAnimatorValueSlider::isTargetDisabled() {
    if(hasTargetAnimator()) {
        return mAnimator->SWT_isDisabled();
    }
    return true;
}

void QrealAnimatorValueSlider::openContextMenu(
        const QPoint &globalPos) {
    if(mAnimator == nullptr) return;
    QMenu menu(this);

    if(mAnimator->prp_isKeyOnCurrentFrame()) {
        menu.addAction("Delete Keyframe",
                       mAnimator.get(),
                       SLOT(anim_deleteCurrentKey()));
    } else {
        menu.addAction("Add Keyframe",
                       mAnimator.get(),
                       SLOT(anim_saveCurrentValueAsKey()));
    }

    menu.addSeparator();

    QAction *recAct = menu.addAction("Recording");
    recAct->setCheckable(true);
    recAct->setChecked(mAnimator->prp_isRecording());
    connect(recAct, SIGNAL(toggled(bool)),
            mAnimator.get(), SLOT(prp_setRecording(bool)));

    QAction *selectedAction = menu.exec(globalPos);
    if(selectedAction == nullptr) {
        return;
    } else {
        MainWindow::getInstance()->callUpdateSchedulers();
    }
}
