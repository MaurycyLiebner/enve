#include "animationbox.h"
#include <QMenu>
#include "GUI/BoxesList/boxsinglewidget.h"
#include "canvas.h"
#include "FileCacheHandlers/animationcachehandler.h"
#include "imagebox.h"
#include "undoredo.h"

AnimationBox::AnimationBox() : BoundingBox(TYPE_IMAGE) {
    setName("Animation");

    setDurationRectangle(SPtrCreate(FixedLenAnimationRect)(this));
    mFrameAnimator = SPtrCreate(IntAnimator)("frame");
}

AnimationBox::~AnimationBox() {
    mSrcFramesCache->removeDependentBox(this);
}

FixedLenAnimationRect *AnimationBox::getAnimationDurationRect() {
    return GetAsPtr(mDurationRectangle, FixedLenAnimationRect);
}

void AnimationBox::updateDurationRectangleAnimationRange() {
    //qreal fpsRatio = getParentCanvas()->getFps()/mFps;
    //qreal timeScale = mTimeScaleAnimator->qra_getCurrentValue()*fpsRatio;

    const auto oldRange = getAnimationDurationRect()->getRelFrameRange();
    int frameCount;
    if(mSrcFramesCache) frameCount = mSrcFramesCache->getFrameCount();
    else frameCount = 0;
    getAnimationDurationRect()->setAnimationFrameDuration(frameCount);
    const auto newRange = getAnimationDurationRect()->getRelFrameRange();

    prp_updateAfterChangedRelFrameRange({oldRange.fMin, newRange.fMin});
    prp_updateAfterChangedRelFrameRange({oldRange.fMax, newRange.fMax});
}

void AnimationBox::reloadCacheHandler() {
    //if(mParentGroup) {
        updateDurationRectangleAnimationRange();
    //}
    if(mFrameRemappingEnabled) {
        int frameCount;
        if(mSrcFramesCache) frameCount = mSrcFramesCache->getFrameCount();
        else frameCount = 1;
        mFrameAnimator->setIntValueRange(0, frameCount - 1);
    }
    reloadSound();
    prp_updateInfluenceRangeAfterChanged();

    //mAnimationCacheHandler->scheduleFrameLoad(mCurrentAnimationFrame);
    scheduleUpdate(Animator::USER_CHANGE);
}

void AnimationBox::setParentGroup(BoxesGroup * const parent) {
    BoundingBox::setParentGroup(parent);
    updateDurationRectangleAnimationRange();
}

bool AnimationBox::shouldScheduleUpdate() {
    if(!mSrcFramesCache || !mParentGroup) return false;
    return BoundingBox::shouldScheduleUpdate();
}

int AnimationBox::getAnimationFrameForRelFrame(const int &relFrame) {
    int pixId;
    const int animStartRelFrame =
                getAnimationDurationRect()->getMinAnimationFrameAsRelFrame();
    if(mFrameRemappingEnabled) {
        pixId = mFrameAnimator->getCurrentIntValueAtRelFrame(relFrame);
    } else {
        pixId = relFrame - animStartRelFrame;
    }

    if(pixId <= 0) {
        pixId = 0;
    } else if(pixId > mSrcFramesCache->getFrameCount() - 1) {
        pixId = mSrcFramesCache->getFrameCount() - 1;
    }

    return pixId;
}
#include "Animators/qrealkey.h"
#include "Animators/effectanimators.h"
void AnimationBox::enableFrameRemapping() {
    if(mFrameRemappingEnabled) return;
    const int frameCount = mSrcFramesCache->getFrameCount();
    mFrameAnimator->setIntValueRange(0, frameCount - 1);
    const int animStartRelFrame =
                getAnimationDurationRect()->getMinAnimationFrameAsRelFrame();
    if(frameCount > 1) {
        const auto firstFrameKey = SPtrCreate(QrealKey)(0, animStartRelFrame,
                                                        mFrameAnimator.get());
        mFrameAnimator->anim_appendKey(firstFrameKey);
        const auto lastFrameKey = SPtrCreate(QrealKey)(frameCount - 1,
                                                       animStartRelFrame + frameCount - 1,
                                                       mFrameAnimator.get());
        mFrameAnimator->anim_appendKey(lastFrameKey);
        mFrameAnimator->anim_setRecordingValue(true);
    } else {
        mFrameAnimator->setCurrentIntValue(0);
    }
    mFrameRemappingEnabled = true;
    ca_prependChildAnimator(mEffectsAnimators.get(), mFrameAnimator);
    prp_updateInfluenceRangeAfterChanged();
    scheduleUpdate(Animator::USER_CHANGE);
}

void AnimationBox::disableFrameRemapping() {
    if(!mFrameRemappingEnabled) return;
    mFrameRemappingEnabled = false;
    mFrameAnimator->anim_removeAllKeys();
    mFrameAnimator->anim_setRecordingValue(false);
    ca_removeChildAnimator(mFrameAnimator);
    prp_updateInfluenceRangeAfterChanged();
    scheduleUpdate(Animator::USER_CHANGE);
}

void AnimationBox::reload() {
    if(mSrcFramesCache) mSrcFramesCache->clearCache();
}

void AnimationBox::anim_setAbsFrame(const int &frame) {
    BoundingBox::anim_setAbsFrame(frame);
    if(!mSrcFramesCache) return;

    mNewCurrentFrameUpdateNeeded = true;

    //if(!mWaitingForSchedulerToBeProcessed) {
        //scheduleUpdate();
    //} else {
        scheduleUpdate(Animator::FRAME_CHANGE);
    //}
}

//void AnimationBox::drawSk(SkCanvas * const canvas) {
//    SkPaint paint;
//    //paint.setFilterQuality(kHigh_SkFilterQuality);
//    canvas->drawImage(mUpdateAnimationImageSk, 0, 0, &paint);
//}
#include "typemenu.h"
void AnimationBox::addActionsToMenu(BoxTypeMenu * const menu) {
    BoundingBox::addActionsToMenu(menu);
    const auto widget = menu->getParentWidget();

    const BoxTypeMenu::PlainOp<AnimationBox> reloadOp =
    [](AnimationBox * box) {
        box->reload();
    };
    menu->addPlainAction("Reload", reloadOp);

    const BoxTypeMenu::PlainOp<AnimationBox> setSrcOp =
    [widget](AnimationBox * box) {
        box->changeSourceFile(widget);
    };
    menu->addPlainAction("Set Source File...", setSrcOp);

    const BoxTypeMenu::CheckOp<AnimationBox> remapOp =
    [](AnimationBox * box, bool checked) {
        if(checked) box->enableFrameRemapping();
        else box->disableFrameRemapping();
    };
    menu->addCheckableAction("Set Source File...",
                             mFrameRemappingEnabled, remapOp);
}

void AnimationBox::setupRenderData(const qreal &relFrame,
                                   BoundingBoxRenderData* data) {
    BoundingBox::setupRenderData(relFrame, data);
    const auto imageData = GetAsPtr(data, AnimationBoxRenderData);
    const int animationFrame = getAnimationFrameForRelFrame(qRound(relFrame));
    imageData->animationFrame = animationFrame;
    const auto upd = mSrcFramesCache->scheduleFrameLoad(animationFrame);
    if(upd) upd->addDependent(imageData);
    else imageData->fImage = mSrcFramesCache->getFrameCopyAtFrame(animationFrame);
}

stdsptr<BoundingBoxRenderData> AnimationBox::createRenderData() {
    return SPtrCreate(AnimationBoxRenderData)(mSrcFramesCache, this);
}

void AnimationBoxRenderData::loadImageFromHandler() {
    fImage = GetAsPtr(fSrcCacheHandler, AnimationCacheHandler)->
            getFrameCopyAtOrBeforeFrame(animationFrame);
}
