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

#include "Animators/qpointfanimator.h"
QrealAnimator* QrealAnimatorValueSlider::getQPointFAnimatorSibling() {
    if(mTarget->SWT_isQrealAnimator()) {
        const auto parent = mTarget->getParent();
        if(parent && parent->SWT_isQPointFAnimator()) {
            const auto qPA = GetAsPtr(parent, QPointFAnimator);
            const bool thisX = qPA->getXAnimator() == mTarget;
            return thisX ? qPA->getYAnimator() :
                           qPA->getXAnimator();
        }
    }
    return nullptr;
}

void QrealAnimatorValueSlider::mouseMoveEvent(QMouseEvent *event) {
    QDoubleSlider::mouseMoveEvent(event);
    if(event->modifiers() & Qt::ShiftModifier) {
        const auto other = getQPointFAnimatorSibling();
        if(!other) return;
        if(!mMouseMoved) other->prp_startTransform();
        const qreal dValue = (event->x() - mPressX)*0.1*mPrefferedValueStep;
        other->incCurrentBaseValue(dValue);
    }
}

bool QrealAnimatorValueSlider::eventFilter(QObject *obj, QEvent *event) {
    const bool keyPress = event->type() == QEvent::KeyPress;
    const bool keyRelease = event->type() == QEvent::KeyRelease;

    if(keyPress || keyRelease) {
        const auto keyEvent = static_cast<QKeyEvent*>(event);
        if(keyEvent->key() == Qt::Key_Shift) {
            const auto other = getQPointFAnimatorSibling();
            if(other) {
                if(keyPress) {
                    if(mMouseMoved) {
                        other->prp_startTransform();
                    }
                } else if(keyRelease) {
                    other->prp_cancelTransform();
                }
            }
        }
    }
    return QDoubleSlider::eventFilter(obj, event);
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
        const auto other = getQPointFAnimatorSibling();
        if(other) other->prp_finishTransform();
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
        bool rec = false;
        bool key = false;
        if(mTarget->SWT_isAnimator()) {
            const auto aTarget = GetAsPtr(mTarget, Animator);
            rec = aTarget->anim_isRecording();
            key = aTarget->anim_getKeyOnCurrentFrame();
        }
        if(rec) {
            const bool disabled = isTargetDisabled() || !isEnabled();
            QDoubleSlider::paint(p,
                                 disabled ? QColor(200, 180, 180) :
                                            QColor(255, 200, 200),
                                 disabled ? QColor(180, 160, 160) :
                                            QColor(255, 160, 160),
                                 key ? (disabled ? QColor(200, 90, 90) : Qt::red) :
                                       (disabled ? Qt::darkGray : Qt::black),
                                 disabled ? Qt::darkGray : Qt::black);
        } else {
            QDoubleSlider::paint(p, !isTargetDisabled() && isEnabled());
        }
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
    return mTarget;
}

bool QrealAnimatorValueSlider::isTargetDisabled() {
    if(hasTarget()) return mTarget->SWT_isDisabled();
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
    if(!selectedAction) return;
    else MainWindow::getInstance()->queScheduledTasksAndUpdate();
}
