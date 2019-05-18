#include "animationbox.h"
#include <QMenu>
#include "GUI/BoxesList/boxsinglewidget.h"
#include "canvas.h"
#include "FileCacheHandlers/animationcachehandler.h"
#include "imagebox.h"
#include "undoredo.h"

AnimationBox::AnimationBox(const BoundingBoxType& type) : BoundingBox(type) {
    setName("Animation");

    setDurationRectangle(SPtrCreate(FixedLenAnimationRect)(this));
    mFrameAnimator = SPtrCreate(IntAnimator)("frame");
}

AnimationBox::~AnimationBox() {
    mSrcFramesCache->removeDependentBox(this);
}

FixedLenAnimationRect *AnimationBox::getAnimationDurationRect() const {
    return GetAsPtr(mDurationRectangle, FixedLenAnimationRect);
}

void AnimationBox::updateDurationRectangleAnimationRange() {
    if(mFrameRemappingEnabled) {
        getAnimationDurationRect()->setAnimationFrameDuration(0);
    } else {
        int frameCount;
        if(mSrcFramesCache) frameCount = mSrcFramesCache->getFrameCount();
        else frameCount = 0;
        const int nFrames = qRound(frameCount*qAbs(mStretch));
        getAnimationDurationRect()->setAnimationFrameDuration(nFrames);
    }
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
    prp_afterWholeInfluenceRangeChanged();

    //mAnimationCacheHandler->scheduleFrameLoad(mCurrentAnimationFrame);
    planScheduleUpdate(Animator::USER_CHANGE);
}

void AnimationBox::setParentGroup(LayerBox * const parent) {
    BoundingBox::setParentGroup(parent);
    updateDurationRectangleAnimationRange();
}

bool AnimationBox::shouldPlanScheduleUpdate() {
    if(!mSrcFramesCache || !mParentGroup) return false;
    return BoundingBox::shouldPlanScheduleUpdate();
}

int AnimationBox::getAnimationFrameForRelFrame(const int &relFrame) {
    const int lastFrameId = mSrcFramesCache->getFrameCount() - 1;
    const int animStartRelFrame =
                getAnimationDurationRect()->getMinAnimationFrameAsRelFrame();
    int pixId;
    if(mFrameRemappingEnabled) {
        pixId = mFrameAnimator->getCurrentIntValueAtRelFrame(relFrame);
    } else {
        if(isZero6Dec(mStretch)) {
            pixId = lastFrameId;
        } else {
            pixId = qRound((relFrame - animStartRelFrame)/qAbs(mStretch));
            if(mStretch < 0) pixId = lastFrameId - pixId;
        }
    }

    if(pixId < 0) pixId = 0;
    else if(pixId > lastFrameId) pixId = lastFrameId;

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
        const int value = frameCount - 1;
        const int frame = animStartRelFrame + frameCount - 1;
        const auto lastFrameKey = SPtrCreate(QrealKey)(value, frame,
                                                       mFrameAnimator.get());
        mFrameAnimator->anim_appendKey(lastFrameKey);
    } else {
        mFrameAnimator->setCurrentIntValue(0);
    }
    mFrameRemappingEnabled = true;
    ca_prependChildAnimator(mEffectsAnimators.get(), mFrameAnimator);
    prp_afterWholeInfluenceRangeChanged();
    planScheduleUpdate(Animator::USER_CHANGE);
    updateDurationRectangleAnimationRange();
}

void AnimationBox::disableFrameRemapping() {
    if(!mFrameRemappingEnabled) return;
    mFrameRemappingEnabled = false;
    mFrameAnimator->anim_removeAllKeys();
    mFrameAnimator->anim_setRecordingValue(false);
    ca_removeChildAnimator(mFrameAnimator);
    prp_afterWholeInfluenceRangeChanged();
    planScheduleUpdate(Animator::USER_CHANGE);
    updateDurationRectangleAnimationRange();
}

void AnimationBox::reload() {
    if(mSrcFramesCache) mSrcFramesCache->clearCache();
}

void AnimationBox::anim_setAbsFrame(const int &frame) {
    BoundingBox::anim_setAbsFrame(frame);
    if(!mSrcFramesCache) return;

    mNewCurrentFrameUpdateNeeded = true;

    //if(!mWaitingForSchedulerToBeProcessed) {
        //planScheduleUpdate();
    //} else {
        planScheduleUpdate(Animator::FRAME_CHANGE);
    //}
}

FrameRange AnimationBox::prp_getIdenticalRelRange(const int &relFrame) const {
    if(isVisibleAndInDurationRect(relFrame)) {
        const auto animDur = getAnimationDurationRect();
        const auto animRange = animDur->getAnimationRange();
        if(animRange.inRange(relFrame))
            return {relFrame, relFrame};
        else if(relFrame > animRange.fMax) {
            const auto baseRange = BoundingBox::prp_getIdenticalRelRange(relFrame);
            const FrameRange durRect{animRange.fMax + 1,
                                     animDur->getRelFrameRange().fMax};
            return baseRange*durRect;
        } else if(relFrame < animRange.fMin) {
            const auto baseRange = BoundingBox::prp_getIdenticalRelRange(relFrame);
            const FrameRange durRect{animDur->getRelFrameRange().fMin,
                                     animRange.fMin - 1};
            return baseRange*durRect;
        }
    }
    return BoundingBox::prp_getIdenticalRelRange(relFrame);
}

//void AnimationBox::drawSk(SkCanvas * const canvas) {
//    SkPaint paint;
//    //paint.setFilterQuality(kHigh_SkFilterQuality);
//    canvas->drawImage(mUpdateAnimationImageSk, 0, 0, &paint);
//}
#include "typemenu.h"
#include <QInputDialog>
void AnimationBox::addActionsToMenu(BoxTypeMenu * const menu) {
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
    menu->addCheckableAction("Frame Remapping",
                             mFrameRemappingEnabled, remapOp);

    const BoxTypeMenu::PlainOp<AnimationBox> stretchOp =
    [this, widget](AnimationBox * box) {
        bool ok = false;
        const int stretch = QInputDialog::getInt(widget,
                                                 "Stretch " + box->getName(),
                                                 "Stretch:",
                                                 qRound(getStretch()*100),
                                                 -1000, 1000, 1, &ok);
        if(!ok) return;
        box->setStretch(stretch*0.01);
    };
    menu->addPlainAction("Stretch...", stretchOp);

    BoundingBox::addActionsToMenu(menu);
}

void AnimationBox::setupRenderData(const qreal &relFrame,
                                   BoundingBoxRenderData * const data) {
    BoundingBox::setupRenderData(relFrame, data);
    const auto imageData = GetAsPtr(data, AnimationBoxRenderData);
    const int animationFrame = getAnimationFrameForRelFrame(qRound(relFrame));
    imageData->fAnimationFrame = animationFrame;
    const auto upd = mSrcFramesCache->scheduleFrameLoad(animationFrame);
    if(upd) upd->addDependent(imageData);
    else imageData->fImage = mSrcFramesCache->getFrameCopyAtFrame(animationFrame);
}

stdsptr<BoundingBoxRenderData> AnimationBox::createRenderData() {
    return SPtrCreate(AnimationBoxRenderData)(mSrcFramesCache, this);
}

void AnimationBoxRenderData::loadImageFromHandler() {
    fImage = GetAsPtr(fSrcCacheHandler, AnimationCacheHandler)->
            getFrameCopyAtOrBeforeFrame(fAnimationFrame);
}
