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

bool KeyFocusTarget::KFT_handleKeyEvent(QKeyEvent *e) {
    const auto func = e->type() == QEvent::KeyRelease ?
                &KeyFocusTarget::KFT_keyReleaseEvent :
                &KeyFocusTarget::KFT_keyPressEvent;

    if(KFT_mCurrentTarget) {
        if((KFT_mCurrentTarget->*func)(e)) {
            KFT_mCurrentTarget->KFT_setFocus();
            return true;
        }
    }
    for(const auto target : KFT_mAllTargets) {
        if(target == KFT_mCurrentTarget) continue;
        if((target->*func)(e)) {
            target->KFT_setFocus();
            return true;
        }
    }

    return false;
}

void KeyFocusTarget::KFT_sTab() {
    if(KFT_mAllTargets.isEmpty()) return;
    const auto lastTarget = KFT_mCurrentTarget ? KFT_mCurrentTarget :
                                                 KFT_mLastTarget;
    if(lastTarget) {
        int index = KFT_mAllTargets.indexOf(lastTarget) + 1;
        if(index >= KFT_mAllTargets.count()) index = 0;
        KFT_mAllTargets.at(index)->KFT_setFocus();
    } else {
        KFT_mAllTargets.first()->KFT_setFocus();
    }
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
