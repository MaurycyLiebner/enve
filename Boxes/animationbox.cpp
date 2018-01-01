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

    setDurationRectangle(new FixedLenAnimationRect(this));
//    mFrameAnimator.blockPointer();
//    mFrameAnimator.setValueRange(0, listOfFrames.count() - 1);
    //    mFrameAnimator.setCurrentIntValue(0);
}

AnimationBox::~AnimationBox() {
    mAnimationCacheHandler->removeDependentBox(this);
    mAnimationCacheHandler = NULL;
}

FixedLenAnimationRect *AnimationBox::getAnimationDurationRect() {
    return ((FixedLenAnimationRect*)mDurationRectangle);
}

void AnimationBox::updateDurationRectangleAnimationRange() {
    //qreal fpsRatio = getParentCanvas()->getFps()/mFps;
    //qreal timeScale = mTimeScaleAnimator->qra_getCurrentValue()*fpsRatio;

    getAnimationDurationRect()->setAnimationFrameDuration(
            qCeil(qAbs(mAnimationCacheHandler->getFramesCount())));
}

void AnimationBox::reloadCacheHandler() {
    //if(mParentGroup != NULL) {
        updateDurationRectangleAnimationRange();
    //}
    reloadSound();
    clearAllCache();

    //mAnimationCacheHandler->scheduleFrameLoad(mCurrentAnimationFrame);
    scheduleUpdate();
}

void AnimationBox::setParentGroup(BoxesGroup *parent) {
    BoundingBox::setParentGroup(parent);
    updateDurationRectangleAnimationRange();
}

bool AnimationBox::shouldScheduleUpdate() {
    if(mAnimationCacheHandler == NULL || mParentGroup == NULL) return false;
    return BoundingBox::shouldScheduleUpdate();
}

int AnimationBox::getAnimationFrameForRelFrame(const int &relFrame) {
    int pixId;
    const int &absMinAnimation =
                getAnimationDurationRect()->getMinAnimationFrameAsRelFrame();
    if(true) {
        pixId = (relFrame - absMinAnimation);
    } else { // reversed
        pixId = mAnimationCacheHandler->getFramesCount() - 1 -
                (relFrame - absMinAnimation);
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
        if(mAnimationCacheHandler != NULL) {
            mAnimationCacheHandler->clearCache();
        }
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
    imageData->image = mAnimationCacheHandler->getFrameAtFrame(animationFrame);
    if(imageData->image == NULL) {
        Updatable *upd = mAnimationCacheHandler->
                scheduleFrameLoad(animationFrame);
        if(upd != NULL) {
            upd->addDependent(imageData);
        }
    }
}

BoundingBoxRenderData *AnimationBox::createRenderData() {
    return new AnimationBoxRenderData(mAnimationCacheHandler, this);
}

void AnimationBoxRenderData::loadImageFromHandler() {
    image = ((AnimationCacheHandler*)srcCacheHandler)->getFrameAtFrame(
                animationFrame);
}
