#include "animationbox.h"
#include <QMenu>
#include "GUI/BoxesList/boxsinglewidget.h"
#include "canvas.h"
#include "FileCacheHandlers/animationcachehandler.h"
#include "imagebox.h"
#include "undoredo.h"

AnimationBox::AnimationBox() :
    BoundingBox(TYPE_IMAGE) {
    setName("Animation");

    setDurationRectangle(SPtrCreate(FixedLenAnimationRect)(this));
    mFrameAnimator = SPtrCreate(IntAnimator)("frame");
}

AnimationBox::~AnimationBox() {
    mAnimationCacheHandler->removeDependentBox(this);
}

FixedLenAnimationRect *AnimationBox::getAnimationDurationRect() {
    return GetAsPtr(mDurationRectangle, FixedLenAnimationRect);
}

void AnimationBox::updateDurationRectangleAnimationRange() {
    //qreal fpsRatio = getParentCanvas()->getFps()/mFps;
    //qreal timeScale = mTimeScaleAnimator->qra_getCurrentValue()*fpsRatio;

    auto oldRange = getAnimationDurationRect()->getRelFrameRange();
    getAnimationDurationRect()->setAnimationFrameDuration(
            mAnimationCacheHandler->getFramesCount());
    auto newRange = getAnimationDurationRect()->getRelFrameRange();

    prp_updateAfterChangedRelFrameRange({oldRange.fMin, newRange.fMin});
    prp_updateAfterChangedRelFrameRange({oldRange.fMax, newRange.fMax});
}

void AnimationBox::reloadCacheHandler() {
    //if(mParentGroup) {
        updateDurationRectangleAnimationRange();
    //}
    if(mFrameRemappingEnabled) {
        const int frameCount = mAnimationCacheHandler->getFramesCount();
        mFrameAnimator->setIntValueRange(0, frameCount - 1);
    }
    reloadSound();
    prp_updateInfluenceRangeAfterChanged();

    //mAnimationCacheHandler->scheduleFrameLoad(mCurrentAnimationFrame);
    scheduleUpdate(Animator::USER_CHANGE);
}

void AnimationBox::setParentGroup(BoxesGroup *parent) {
    BoundingBox::setParentGroup(parent);
    updateDurationRectangleAnimationRange();
}

bool AnimationBox::shouldScheduleUpdate() {
    if(mAnimationCacheHandler == nullptr || mParentGroup == nullptr) return false;
    return BoundingBox::shouldScheduleUpdate();
}

int AnimationBox::getAnimationFrameForRelFrame(const int &relFrame) {
    int pixId;
    const int animStartRelFrame =
                getAnimationDurationRect()->getMinAnimationFrameAsRelFrame();
    if(mFrameRemappingEnabled) {
        pixId = mFrameAnimator->getCurrentIntValueAtRelFrame(relFrame);
    } else { // reversed
        pixId = (relFrame - animStartRelFrame);
    }

    if(pixId <= 0) {
        pixId = 0;
    } else if(pixId > mAnimationCacheHandler->getFramesCount() - 1) {
        pixId = mAnimationCacheHandler->getFramesCount() - 1;
    }

    return pixId;
}
#include "Animators/qrealkey.h"
#include "Animators/effectanimators.h"
void AnimationBox::enableFrameRemapping() {
    if(mFrameRemappingEnabled) return;
    const int frameCount = mAnimationCacheHandler->getFramesCount();
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

void AnimationBox::anim_setAbsFrame(const int &frame) {
    BoundingBox::anim_setAbsFrame(frame);
    if(!mAnimationCacheHandler) return;

    mNewCurrentFrameUpdateNeeded = true;

    //if(!mWaitingForSchedulerToBeProcessed) {
        //scheduleUpdate();
    //} else {
        scheduleUpdate(Animator::FRAME_CHANGE);
    //}
}

//void AnimationBox::drawSk(SkCanvas *canvas) {
//    SkPaint paint;
//    //paint.setFilterQuality(kHigh_SkFilterQuality);
//    canvas->drawImage(mUpdateAnimationImageSk, 0, 0, &paint);
//}

void AnimationBox::addActionsToMenu(QMenu * const menu,
                                    QWidget* const widgetsParent) {
    menu->addAction("Reload", [this]() {
        if(mAnimationCacheHandler) {
            mAnimationCacheHandler->clearCache();
        }
    });

    menu->addAction("Set Source File...", [this, widgetsParent]() {
        changeSourceFile(widgetsParent);
    });

    const auto fR = menu->addAction("Frame Remapping", [this]() {
        if(this->mFrameRemappingEnabled) {
            this->disableFrameRemapping();
        } else {
            this->enableFrameRemapping();
        }
    });
    fR->setCheckable(true);
    fR->setChecked(mFrameRemappingEnabled);
}

void AnimationBox::setupBoundingBoxRenderDataForRelFrameF(
                                const qreal &relFrame,
                                BoundingBoxRenderData* data) {
    BoundingBox::setupBoundingBoxRenderDataForRelFrameF(relFrame,
                                                       data);
    auto imageData = GetAsPtr(data, AnimationBoxRenderData);
    int animationFrame = getAnimationFrameForRelFrame(qRound(relFrame));
    imageData->animationFrame = animationFrame;
    imageData->fImage = mAnimationCacheHandler->getFrameCopyAtFrame(animationFrame);
    if(!imageData->fImage) {
        auto upd = mAnimationCacheHandler->scheduleFrameLoad(animationFrame);
        if(upd) {
            upd->addDependent(imageData);
        }
    }
}

stdsptr<BoundingBoxRenderData> AnimationBox::createRenderData() {
    return SPtrCreate(AnimationBoxRenderData)(mAnimationCacheHandler, this);
}

void AnimationBoxRenderData::loadImageFromHandler() {
    fImage = GetAsPtr(srcCacheHandler, AnimationCacheHandler)->
            getFrameCopyAtOrBeforeFrame(animationFrame);
}
