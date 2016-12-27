#include "boundingbox.h"
#include "boxesgroup.h"
#include "undoredo.h"
#include <QPainter>
#include "keysview.h"

void BoundingBox::selectionChangeTriggered(bool shiftPressed) {
    if(shiftPressed) {
        if(mSelected) {
            mParent->removeBoxFromSelection(this);
        } else {
            mParent->addBoxToSelection(this);
        }
    } else {
        mParent->clearBoxesSelection();
        mParent->addBoxToSelection(this);
    }
}

void BoundingBox::addEffect(PixmapEffect *effect) {
    effect->setUpdater(new PixmapEffectUpdater(this));
    effect->incNumberPointers();
    mEffects << effect;

    if(!mEffectsAnimators.hasChildAnimators()) {
        mAnimatorsCollection.addAnimator(&mEffectsAnimators);
        addActiveAnimator(&mEffectsAnimators);
    }
    mEffectsAnimators.addChildAnimator(effect);
}

void BoundingBox::removeEffect(PixmapEffect *effect) {

    mEffects.removeOne(effect);
    removeActiveAnimator(effect);
    mEffectsAnimators.removeChildAnimator(effect);
    if(!mEffectsAnimators.hasChildAnimators()) {
        mAnimatorsCollection.removeAnimator(&mEffectsAnimators);
        removeActiveAnimator(&mEffectsAnimators);
    }
    effect->decNumberPointers();
}

QrealAnimator *BoundingBox::getAnimatorsCollection() {
    return &mAnimatorsCollection;
}

void BoundingBox::addActiveAnimator(QrealAnimator *animator)
{
    mActiveAnimators << animator;
    emit addActiveAnimatorSignal(animator);
}

void BoundingBox::removeActiveAnimator(QrealAnimator *animator)
{
    mActiveAnimators.removeOne(animator);
    emit removeActiveAnimatorSignal(animator);
}

void BoundingBox::drawKeys(QPainter *p,
                           qreal pixelsPerFrame,
                           qreal drawY,
                           int startFrame, int endFrame) {
    mAnimatorsCollection.drawKeys(p,
                                  pixelsPerFrame, drawY,
                                  startFrame, endFrame);
}

void BoundingBox::setName(QString name)
{
    mName = name;
}

QString BoundingBox::getName()
{
    return mName;
}

void BoundingBox::setVisibile(bool visible, bool saveUndoRedo) {
    if(mVisible == visible) return;
    if(mSelected) {
        ((BoxesGroup*) mParent)->removeBoxFromSelection(this);
    }
    if(saveUndoRedo) {
        addUndoRedo(new SetBoxVisibleUndoRedo(this, mVisible, visible));
    }
    mVisible = visible;

    scheduleAwaitUpdate();
}

void BoundingBox::hide()
{
    setVisibile(false);
}

void BoundingBox::show()
{
    setVisibile(true);
}

bool BoundingBox::isVisibleAndUnlocked() {
    return mVisible && !mLocked;
}

bool BoundingBox::isVisible()
{
    return mVisible;
}

bool BoundingBox::isLocked() {
    return mLocked;
}

void BoundingBox::lock() {
    setLocked(true);
}

void BoundingBox::unlock() {
    setLocked(false);
}

void BoundingBox::setLocked(bool bt) {
    if(bt == mLocked) return;
    if(mSelected) {
        ((BoxesGroup*) mParent)->removeBoxFromSelection(this);
    }
    mLocked = bt;
}
