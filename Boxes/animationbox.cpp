#include "animationbox.h"
#include "BoxesList/boxsinglewidget.h"
#include "Animators/animatorupdater.h"
#include "canvas.h"
#include "filesourcescache.h"
#include "imagebox.h"
#include "undoredo.h"

AnimationBox::AnimationBox() :
    BoundingBox(TYPE_IMAGE) {
    setName("Animation");
    mTimeScaleAnimator->prp_setName("time scale");
    mTimeScaleAnimator->qra_setValueRange(-100, 100);
    mTimeScaleAnimator->qra_setCurrentValue(1.);
    mTimeScaleAnimator->setPrefferedValueStep(0.05);
    mTimeScaleAnimator->prp_setUpdater(new AnimationBoxFrameUpdater(this));
    mTimeScaleAnimator->prp_blockUpdater();
    ca_addChildAnimator(mTimeScaleAnimator.data());

    setDurationRectangle(new FixedLenAnimationRect(this));
//    mFrameAnimator.blockPointer();
//    mFrameAnimator.setValueRange(0, listOfFrames.count() - 1);
//    mFrameAnimator.setCurrentIntValue(0);
}

void AnimationBox::makeDuplicate(Property *targetBox) {
    BoundingBox::makeDuplicate(targetBox);
    AnimationBox *animationBoxTarget = (AnimationBox*)targetBox;
    animationBoxTarget->duplicateAnimationBoxAnimatorsFrom(
                mTimeScaleAnimator.data());
}

void AnimationBox::duplicateAnimationBoxAnimatorsFrom(
        QrealAnimator *timeScaleAnimator) {
    timeScaleAnimator->makeDuplicate(mTimeScaleAnimator.data());
}

//BoundingBox *AnimationBox::createNewDuplicate(BoxesGroup *parent) {
//    return new AnimationBox(parent);
//}

FixedLenAnimationRect *AnimationBox::getAnimationDurationRect() {
    return ((FixedLenAnimationRect*)mDurationRectangle);
}

void AnimationBox::updateDurationRectangleAnimationRange() {
    qreal fpsRatio = getParentCanvas()->getFps()/mFps;
    qreal timeScale = mTimeScaleAnimator->qra_getCurrentValue()*fpsRatio;

    getAnimationDurationRect()->setAnimationFrameDuration(
            qCeil(qAbs(timeScale*mAnimationCacheHandler->getFramesCount())));
}

void AnimationBox::reloadFile() {
    if(mParent != NULL) {
        updateDurationRectangleAnimationRange();
    }
    reloadSound();
    clearAllCache();

    //mAnimationCacheHandler->scheduleFrameLoad(mCurrentAnimationFrame);
    scheduleUpdate();
}

void AnimationBox::setParent(BoxesGroup *parent) {
    mParent = parent->ref<BoxesGroup>();
    mTransformAnimator->setParentTransformAnimator(
                        mParent->getTransformAnimator());

    updateDurationRectangleAnimationRange();

    updateCombinedTransform();
}

bool AnimationBox::shouldScheduleUpdate() {
    if(mAnimationCacheHandler == NULL || mParent == NULL) return false;
    return BoundingBox::shouldScheduleUpdate();
}

int AnimationBox::getAnimationFrameForRelFrame(const int &relFrame) {
    qreal fpsRatio = getParentCanvas()->getFps()/mFps;
    qreal timeScale = mTimeScaleAnimator->qra_getCurrentValue()*fpsRatio;

    int pixId;
    const int &absMinAnimation =
                getAnimationDurationRect()->getMinAnimationFrameAsRelFrame();
    if(timeScale > 0.) {
        pixId = (relFrame - absMinAnimation)/timeScale;
    } else {
        pixId = mAnimationCacheHandler->getFramesCount() - 1 +
                (relFrame - absMinAnimation)/timeScale;
    }

    if(pixId <= 0) {
        pixId = 0;
    } else if(pixId > mAnimationCacheHandler->getFramesCount() - 1){
        pixId = mAnimationCacheHandler->getFramesCount() - 1;
    }

    return pixId;
}

void AnimationBox::prp_setAbsFrame(const int &frame) {
    BoundingBox::prp_setAbsFrame(frame);
    if(mAnimationCacheHandler == NULL) return;

    mNewCurrentFrameUpdateNeeded = true;

    //if(!mWaitingForSchedulerToBeProcessed) {
        //scheduleUpdate();
    //} else {
        scheduleUpdate();
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

bool AnimationBox::handleSelectedCanvasAction(QAction *selectedAction) {
    if(selectedAction->objectName() == "ab_set_src_file") {
        changeSourceFile();
    } else if(selectedAction->objectName() == "ab_reload") {
        reloadFile();
    } else {
        return false;
    }
    return true;
}

void AnimationBox::setupBoundingBoxRenderDataForRelFrame(
                                const int &relFrame,
                                BoundingBoxRenderData *data) {
    BoundingBox::setupBoundingBoxRenderDataForRelFrame(relFrame,
                                                       data);
    AnimationBoxRenderData *imageData = (AnimationBoxRenderData*)data;
    int animationFrame = getAnimationFrameForRelFrame(relFrame);
    imageData->animationFrame = animationFrame;
    imageData->image = mAnimationCacheHandler->getFrameAtFrame(
                                    animationFrame);
    if(imageData->image == NULL) {
        if(mAnimationCacheHandler->getFrameAtFrame(
                    animationFrame).get() == NULL) {
            mAnimationCacheHandler->scheduleFrameLoad(animationFrame)->
                    addDependent(imageData);
        }
    }
}

BoundingBoxRenderData *AnimationBox::createRenderData() {
    return new AnimationBoxRenderData(mAnimationCacheHandler, this);
}

void AnimationBoxRenderData::loadImageFromHandler() {
    image = ((AnimationCacheHandler*)srcCacheHandler)->
            getFrameAtFrame(relFrame);
}
