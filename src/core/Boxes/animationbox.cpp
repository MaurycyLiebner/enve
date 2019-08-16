#include "animationbox.h"
#include <QMenu>
#include "canvas.h"
#include "FileCacheHandlers/animationcachehandler.h"
#include "imagebox.h"
#include "undoredo.h"
#include "Animators/qrealkey.h"
#include "Animators/rastereffectanimators.h"

AnimationBox::AnimationBox(const eBoxType type) : BoundingBox(type) {
    connect(this, &eBoxOrSound::parentChanged,
            this, &AnimationBox::updateDurationRectangleAnimationRange);

    prp_setName("Animation");

    setDurationRectangle(enve::make_shared<FixedLenAnimationRect>(this));
    mDurationRectangleLocked = true;

    mFrameAnimator = enve::make_shared<IntAnimator>("frame");
    ca_prependChildAnimator(mRasterEffectsAnimators.get(), mFrameAnimator);
    mFrameAnimator->SWT_hide();
}

FixedLenAnimationRect *AnimationBox::getAnimationDurationRect() const {
    return static_cast<FixedLenAnimationRect*>(mDurationRectangle.get());
}

void AnimationBox::updateDurationRectangleAnimationRange() {
    if(mFrameRemappingEnabled) {
        const int nFrames = getAnimationDurationRect()->getFrameDuration();
        getAnimationDurationRect()->setAnimationFrameDuration(0);
        getAnimationDurationRect()->setFramesDuration(nFrames);
    } else {
        int frameCount;
        if(mSrcFramesCache) frameCount = mSrcFramesCache->getFrameCount();
        else frameCount = 0;
        const int nFrames = qRound(frameCount*qAbs(mStretch));
        getAnimationDurationRect()->setAnimationFrameDuration(nFrames);
    }
}

void AnimationBox::animationDataChanged() {
    //if(mParentGroup) {
    updateDurationRectangleAnimationRange();
    //}
    if(mFrameRemappingEnabled) {
        int frameCount;
        if(mSrcFramesCache) frameCount = mSrcFramesCache->getFrameCount();
        else frameCount = 1;
        mFrameAnimator->setIntValueRange(0, frameCount - 1);
    }
    prp_afterWholeInfluenceRangeChanged();
}

bool AnimationBox::shouldScheduleUpdate() {
    if(!mSrcFramesCache) return false;
    return BoundingBox::shouldScheduleUpdate();
}

int AnimationBox::getAnimationFrameForRelFrame(const qreal relFrame) {
    const int lastFrameId = mSrcFramesCache->getFrameCount() - 1;
    const int animStartRelFrame =
                getAnimationDurationRect()->getMinAnimationFrameAsRelFrame();
    int pixId;
    if(mFrameRemappingEnabled) {
        pixId = mFrameAnimator->getBaseIntValue(relFrame);
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

void AnimationBox::enableFrameRemappingAction() {
    if(mFrameRemappingEnabled) return;
    const int frameCount = mSrcFramesCache->getFrameCount();
    mFrameAnimator->setIntValueRange(0, frameCount - 1);
    const int animStartRelFrame =
                getAnimationDurationRect()->getMinAnimationFrameAsRelFrame();
    if(frameCount > 1) {
        const auto firstFrameKey = enve::make_shared<QrealKey>(0, animStartRelFrame,
                                                        mFrameAnimator.get());
        mFrameAnimator->anim_appendKey(firstFrameKey);
        const int value = frameCount - 1;
        const int frame = animStartRelFrame + frameCount - 1;
        const auto lastFrameKey = enve::make_shared<QrealKey>(value, frame,
                                                       mFrameAnimator.get());
        mFrameAnimator->anim_appendKey(lastFrameKey);
    } else {
        mFrameAnimator->setCurrentIntValue(0);
    }
    enableFrameRemapping();
    prp_afterWholeInfluenceRangeChanged();
    planScheduleUpdate(UpdateReason::userChange);
    updateDurationRectangleAnimationRange();
}

void AnimationBox::enableFrameRemapping() {
    if(mFrameRemappingEnabled) return;
    mFrameRemappingEnabled = true;
    mFrameAnimator->SWT_show();
}

void AnimationBox::disableFrameRemapping() {
    if(!mFrameRemappingEnabled) return;
    mFrameRemappingEnabled = false;
    mFrameAnimator->anim_removeAllKeys();
    mFrameAnimator->anim_setRecordingValue(false);
    mFrameAnimator->SWT_hide();
    prp_afterWholeInfluenceRangeChanged();
    planScheduleUpdate(UpdateReason::userChange);
    updateDurationRectangleAnimationRange();
}

void AnimationBox::reload() {
    if(mSrcFramesCache) mSrcFramesCache->reload();
}

void AnimationBox::anim_setAbsFrame(const int frame) {
    BoundingBox::anim_setAbsFrame(frame);
    if(!mSrcFramesCache) return;

    mNewCurrentFrameUpdateNeeded = true;

    //if(!mWaitingForSchedulerToBeProcessed) {
        //planScheduleUpdate();
    //} else {
        planScheduleUpdate(UpdateReason::frameChange);
    //}
}

FrameRange AnimationBox::prp_getIdenticalRelRange(const int relFrame) const {
    if(isVisibleAndInDurationRect(relFrame) && !mFrameRemappingEnabled) {
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
void AnimationBox::setupCanvasMenu(PropertyMenu * const menu) {
    const auto widget = menu->getParentWidget();

    const PropertyMenu::PlainSelectedOp<AnimationBox> reloadOp =
    [](AnimationBox * box) {
        box->reload();
    };
    menu->addPlainAction("Reload", reloadOp);

    const PropertyMenu::PlainSelectedOp<AnimationBox> setSrcOp =
    [widget](AnimationBox * box) {
        box->changeSourceFile(widget);
    };
    menu->addPlainAction("Set Source File...", setSrcOp);

    const PropertyMenu::CheckSelectedOp<AnimationBox> remapOp =
    [](AnimationBox * box, bool checked) {
        if(checked) box->enableFrameRemappingAction();
        else box->disableFrameRemapping();
    };
    menu->addCheckableAction("Frame Remapping",
                             mFrameRemappingEnabled, remapOp);

    const PropertyMenu::PlainSelectedOp<AnimationBox> stretchOp =
    [this, widget](AnimationBox * box) {
        bool ok = false;
        const int stretch = QInputDialog::getInt(widget,
                                                 "Stretch " + box->prp_getName(),
                                                 "Stretch:",
                                                 qRound(getStretch()*100),
                                                 -1000, 1000, 1, &ok);
        if(!ok) return;
        box->setStretch(stretch*0.01);
    };
    menu->addPlainAction("Stretch...", stretchOp);

    BoundingBox::setupCanvasMenu(menu);
}

void AnimationBox::writeBoundingBox(QIODevice * const target) {
    BoundingBox::writeBoundingBox(target);
    target->write(rcConstChar(&mFrameRemappingEnabled), sizeof(bool));
}

void AnimationBox::readBoundingBox(QIODevice * const target) {
    BoundingBox::readBoundingBox(target);
    bool frameRemapping;
    target->read(rcChar(&frameRemapping), sizeof(bool));
    if(frameRemapping) enableFrameRemapping();
    else disableFrameRemapping();
}

void AnimationBox::setupRenderData(const qreal relFrame,
                                   BoxRenderData * const data) {
    BoundingBox::setupRenderData(relFrame, data);
    const auto imageData = static_cast<AnimationBoxRenderData*>(data);
    const int animationFrame = getAnimationFrameForRelFrame(relFrame);
    imageData->fAnimationFrame = animationFrame;
    const auto upd = mSrcFramesCache->scheduleFrameLoad(animationFrame);
    if(upd) upd->addDependent(imageData);
    else imageData->fImage = mSrcFramesCache->getFrameCopyAtFrame(animationFrame);
}

stdsptr<BoxRenderData> AnimationBox::createRenderData() {
    return enve::make_shared<AnimationBoxRenderData>(mSrcFramesCache.get(), this);
}

void AnimationBoxRenderData::loadImageFromHandler() {
    fImage = fSrcCacheHandler->getFrameCopyAtOrBeforeFrame(fAnimationFrame);
}
