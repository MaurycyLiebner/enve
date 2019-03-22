#include "qrealanimatorvalueslider.h"
#include <QMenu>
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
    setTarget(animator);
}

QrealAnimatorValueSlider::QrealAnimatorValueSlider(QrealAnimator *animator,
                                                   QWidget *parent) :
    QDoubleSlider(parent) {
    setTarget(animator);
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
    if(mTarget) {
        mBlockAnimatorSignals = true;
        mTarget->prp_startTransform();
    }
    QDoubleSlider::emitEditingStarted(value);
}

void QrealAnimatorValueSlider::emitValueChangedExternal(qreal value) {
    setDisplayedValue(value);
    emitValueChanged(value);
}

void QrealAnimatorValueSlider::emitValueChanged(qreal value) {
    if(mTarget) {
        if(mTarget->SWT_isQrealAnimator()) {
            GetAsPtr(mTarget, QrealAnimator)->setCurrentBaseValue(value);
        } else if(mTarget->SWT_isIntProperty()) {
            GetAsPtr(mTarget, IntProperty)->setCurrentValue(qRound(value));
        }
    }
    QDoubleSlider::emitValueChanged(value);
}

void QrealAnimatorValueSlider::setValueExternal(qreal value) {
    if(mTarget) {
        mBlockAnimatorSignals = true;
        if(mTarget->SWT_isQrealAnimator()) {
            GetAsPtr(mTarget, QrealAnimator)->setCurrentBaseValue(value);
        } else if(mTarget->SWT_isIntProperty()) {
            GetAsPtr(mTarget, IntProperty)->setCurrentValue(qRound(value));
        }
        mBlockAnimatorSignals = false;
    }
    setDisplayedValue(value);
}

void QrealAnimatorValueSlider::emitEditingFinished(qreal value) {
    if(mTarget) {
        mTarget->prp_finishTransform();
        mBlockAnimatorSignals = false;
    }
    QDoubleSlider::emitEditingFinished(value);
}

void QrealAnimatorValueSlider::nullifyAnimator() {
    clearTarget();
}

void QrealAnimatorValueSlider::setValueFromAnimator(qreal val) {
    if(mBlockAnimatorSignals) return;
    setDisplayedValue(val);
    emit displayedValueChanged(val);
}

void QrealAnimatorValueSlider::paint(QPainter *p) {
    if(!mTarget) {
        QDoubleSlider::paint(p);
    } else {
        if(isTargetDisabled()) p->setOpacity(.5);
        bool rec = false;
        bool key = false;
        if(mTarget->SWT_isAnimator()) {
            const auto aTarget = GetAsPtr(mTarget, Animator);
            rec = aTarget->anim_isRecording();
            key = aTarget->anim_getKeyOnCurrentFrame();
        }
        QDoubleSlider::paint(p,
                    rec ? QColor(255, 200, 200) : QColor(255, 255, 255),
                    rec ? QColor(255, 160, 160) : QColor(220, 220, 220),
                    rec && key ? Qt::red : Qt::black);
        p->setOpacity(1);
    }
}

void QrealAnimatorValueSlider::clearTarget() {
    if(mTarget) disconnect(mTarget, nullptr, this, nullptr);
    mTarget = nullptr;
}

void QrealAnimatorValueSlider::setTarget(QrealAnimator * const animator) {
    if(animator == mTarget) return;
    clearTarget();
    mTarget = animator;
    if(mTarget) {
        setNumberDecimals(animator->getNumberDecimals());
        connect(animator, &QrealAnimator::valueChangedSignal,
                this, &QrealAnimatorValueSlider::setValueFromAnimator);
        connect(animator, &QrealAnimator::beingDeleted,
                this, &QrealAnimatorValueSlider::nullifyAnimator);

        setValueRange(animator->getMinPossibleValue(),
                      animator->getMaxPossibleValue());
        setPrefferedValueStep(animator->getPrefferedValueStep());

        setDisplayedValue(animator->getCurrentBaseValue());
    }
}

void QrealAnimatorValueSlider::setTarget(IntProperty * const animator) {
    if(animator == mTarget) return;
    clearTarget();
    if(animator) {
        setNumberDecimals(0);
        connect(animator, &IntProperty::valueChangedSignal,
                this, &QrealAnimatorValueSlider::setValueFromAnimator);
        connect(animator, &IntProperty::beingDeleted,
                this, &QrealAnimatorValueSlider::nullifyAnimator);

        setValueRange(animator->getMinValue(),
                      animator->getMaxValue());
        setPrefferedValueStep(1);

        setDisplayedValue(animator->getValue());
        mTarget = animator;
    }
}

bool QrealAnimatorValueSlider::hasTarget() {
    return mTarget != nullptr;
}

bool QrealAnimatorValueSlider::isTargetDisabled() {
    if(hasTarget()) {
        return mTarget->SWT_isDisabled();
    }
    return true;
}

void QrealAnimatorValueSlider::openContextMenu(
        const QPoint &globalPos) {
    if(!mTarget) return;
    if(!mTarget->SWT_isAnimator()) return;
    const auto aTarget = GetAsPtr(mTarget, Animator);
    QMenu menu(this);

    if(aTarget->anim_getKeyOnCurrentFrame()) {
        menu.addAction("Delete Keyframe",
                       aTarget,
                       &Animator::anim_deleteCurrentKey);
    } else {
        menu.addAction("Add Keyframe",
                       aTarget,
                       &Animator::anim_saveCurrentValueAsKey);
    }

    menu.addSeparator();

    QAction * const recAct = menu.addAction("Recording");
    recAct->setCheckable(true);
    recAct->setChecked(aTarget->anim_isRecording());
    connect(recAct, &QAction::toggled,
            aTarget, &Animator::anim_setRecording);

    QAction * const selectedAction = menu.exec(globalPos);
    if(!selectedAction) {
        return;
    } else {
        MainWindow::getInstance()->queScheduledTasksAndUpdate();
    }
}
