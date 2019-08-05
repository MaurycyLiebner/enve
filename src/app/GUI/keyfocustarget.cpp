#include "keyfocustarget.h"
KeyFocusTarget *KeyFocusTarget::KFT_mCurrentTarget = nullptr;
KeyFocusTarget *KeyFocusTarget::KFT_mLastTarget = nullptr;
QList<KeyFocusTarget*> KeyFocusTarget::KFT_mAllTargets;

KeyFocusTarget::KeyFocusTarget() {
    KFT_mAllTargets << this;
}

KeyFocusTarget::~KeyFocusTarget() {
    KFT_mAllTargets.removeOne(this);
    if(KFT_mLastTarget == this)
        KFT_mLastTarget = nullptr;
    if(KFT_mCurrentTarget == this) {
        KFT_mCurrentTarget = nullptr;
        KFT_sSetRandomTarget();
    }
}

KeyFocusTarget * KeyFocusTarget::KFT_getCurrentTarget() {
    return KeyFocusTarget::KFT_mCurrentTarget;
}

#include <QDebug>
bool KeyFocusTarget::KFT_handleKeyEvent(QKeyEvent *e) {
    if(KFT_mCurrentTarget) {
        if(KFT_mCurrentTarget->KFT_handleKeyEventForTarget(e)) {
            KFT_mCurrentTarget->KFT_setFocus();
            return true;
        }
    }
    for(const auto target : KFT_mAllTargets) {
        if(target == KFT_mCurrentTarget) continue;
        if(target->KFT_handleKeyEventForTarget(e)) {
            target->KFT_setFocus();
            return true;
        }
    }

    return false;
}

void KeyFocusTarget::KFT_sSetRandomTarget() {
    if(KFT_mCurrentTarget) return KFT_mCurrentTarget->KFT_setFocus();
    if(KFT_mAllTargets.isEmpty()) return;
    if(KFT_mLastTarget) KFT_sSetLastTarget();
    else KFT_mAllTargets.first()->KFT_setFocus();
}

void KeyFocusTarget::KFT_sSetLastTarget() {
    if(KFT_mLastTarget) KFT_mLastTarget->KFT_setFocus();
}

void KeyFocusTarget::KFT_setCurrentTarget(KeyFocusTarget *target) {
    if(target == KFT_mCurrentTarget) return;
    if(KFT_mCurrentTarget) KFT_mCurrentTarget->KFT_clearFocus();
    KFT_mCurrentTarget = target;
    if(target) KFT_mLastTarget = target;
}
