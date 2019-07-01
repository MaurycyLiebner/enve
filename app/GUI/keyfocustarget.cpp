#include "keyfocustarget.h"
KeyFocusTarget *KeyFocusTarget::KFT_mCurrentTarget = nullptr;
QList<KeyFocusTarget*> KeyFocusTarget::KFT_mAllTargets;

KeyFocusTarget::KeyFocusTarget() {
    KFT_mAllTargets << this;
}

KeyFocusTarget::~KeyFocusTarget() {
    KFT_mAllTargets.removeOne(this);
    if(KFT_mCurrentTarget == this)
        KFT_mCurrentTarget = nullptr;
}

KeyFocusTarget * KeyFocusTarget::KFT_getCurrentTarget() {
    return KeyFocusTarget::KFT_mCurrentTarget;
}

#include <QDebug>
bool KeyFocusTarget::KFT_handleKeyEvent(QKeyEvent *e) {
    if(KFT_mCurrentTarget) {
        if(KFT_mCurrentTarget->KFT_handleKeyEventForTarget(e)) {
            return true;
        }
    }
    for(KeyFocusTarget *target : KFT_mAllTargets) {
        if(target == KFT_mCurrentTarget) continue;
        if(target->KFT_handleKeyEventForTarget(e)) {
            target->KFT_setFocus();
            return true;
        }
    }

    return false;
}

void KeyFocusTarget::KFT_setCurrentTarget(KeyFocusTarget *target) {
    if(target == KFT_mCurrentTarget) return;
    if(KFT_mCurrentTarget)
        KFT_mCurrentTarget->KFT_clearFocus();
    KFT_mCurrentTarget = target;
}
