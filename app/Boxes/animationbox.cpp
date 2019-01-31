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
//    mFrameAnimator.blockPointer();
//    mFrameAnimator.setValueRange(0, listOfFrames.count() - 1);
    //    mFrameAnimator.setCurrentIntValue(0);
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
    reloadSound();
    clearAllCache();

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
    const int &absMinAnimation =
                getAnimationDurationRect()->getMinAnimationFrameAsRelFrame();
//    if(true) {
        pixId = (relFrame - absMinAnimation);
//    } else { // reversed
//        pixId = mAnimationCacheHandler->getFramesCount() - 1 -
//                (relFrame - absMinAnimation);
//    }

    if(pixId <= 0) {
        pixId = 0;
    } else if(pixId > mAnimationCacheHandler->getFramesCount() - 1) {
        pixId = mAnimationCacheHandler->getFramesCount() - 1;
    }

    return pixId;
}

void AnimationBox::prp_setAbsFrame(const int &frame) {
    BoundingBox::prp_setAbsFrame(frame);
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

void AnimationBox::addActionsToMenu(QMenu *menu) {
    menu->addAction("Reload")->setObjectName("ab_reload");
    menu->addAction("Set Source File...")->
            setObjectName("ab_set_src_file");
}

bool AnimationBox::handleSelectedCanvasAction(QAction *selectedAction,
                                              QWidget* widgetsParent) {
    if(selectedAction->objectName() == "ab_set_src_file") {
        changeSourceFile(widgetsParent);
    } else if(selectedAction->objectName() == "ab_reload") {
        if(mAnimationCacheHandler) {
            mAnimationCacheHandler->clearCache();
        }
    } else {
        return false;
    }
    return true;
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
