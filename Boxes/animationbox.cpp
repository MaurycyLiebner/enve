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
        updateCurrentAnimationFrame();
    }
    reloadSound();
    clearAllCache();

    //mAnimationCacheHandler->scheduleFrameLoad(mCurrentAnimationFrame);
    scheduleSoftUpdate();
}

void AnimationBox::setParent(BoxesGroup *parent) {
    mParent = parent->ref<BoxesGroup>();
    mTransformAnimator->setParentTransformAnimator(
                        mParent->getTransformAnimator());

    updateDurationRectangleAnimationRange();
    updateCurrentAnimationFrame();

    updateCombinedTransform();
}

void AnimationBox::schedulerProccessed() {
    //mUpdateAnimationFrame = mCurrentAnimationFrame;
    qDebug() << "animationbox scheduler processed ";
    BoundingBox::schedulerProccessed();
}

void AnimationBox::scheduleUpdate() {
    if(mAnimationCacheHandler == NULL || mParent == NULL) return;
    mUpdateAnimationFrame = mCurrentAnimationFrame;
    if(mAnimationCacheHandler->getFrameAtFrame(
                mUpdateAnimationFrame).get() == NULL) {
        mAnimationCacheHandler->scheduleFrameLoad(mUpdateAnimationFrame)->
                addDependent(this);
    }
    mNewCurrentFrameUpdateNeeded = false;
    BoundingBox::scheduleUpdate();
}

void AnimationBox::afterUpdate() {
    BoundingBox::afterUpdate();
    if(mNewCurrentFrameUpdateNeeded) {
        scheduleSoftUpdate();
    }
}

void AnimationBox::updateCurrentAnimationFrame() {
    qreal fpsRatio = getParentCanvas()->getFps()/mFps;
    qreal timeScale = mTimeScaleAnimator->qra_getCurrentValue()*fpsRatio;

    int pixId;
    const int &absMinAnimation =
                getAnimationDurationRect()->getMinAnimationFrameAsAbsFrame();
    if(timeScale > 0.) {
        pixId = (anim_mCurrentAbsFrame - absMinAnimation)/timeScale;
    } else {
        pixId = mAnimationCacheHandler->getFramesCount() - 1 +
                (anim_mCurrentAbsFrame - absMinAnimation)/timeScale;
    }

    if(pixId <= 0) {
        pixId = 0;
    } else if(pixId > mAnimationCacheHandler->getFramesCount() - 1){
        pixId = mAnimationCacheHandler->getFramesCount() - 1;
    }

    if(mCurrentAnimationFrame == pixId) return;
    mCurrentAnimationFrame = pixId;
}

void AnimationBox::prp_setAbsFrame(const int &frame) {
    BoundingBox::prp_setAbsFrame(frame);
    if(mAnimationCacheHandler == NULL) return;
    updateCurrentAnimationFrame();
    if(mAnimationCacheHandler->getFrameAtFrame(
                mCurrentAnimationFrame).get() != NULL) {
        mUpdateAnimationFrame = mCurrentAnimationFrame;
    }
    mNewCurrentFrameUpdateNeeded = true;

    //if(!mWaitingForSchedulerToBeProcessed) {
        //scheduleUpdate();
    //} else {
        scheduleSoftUpdate();
    //}
}

void AnimationBox::afterSuccessfulUpdate() {
    sk_sp<SkImage> image =
            ((ImageBoxRenderData*)mCurrentRenderData.get())->image;
    mRelBoundingRect = QRectF(0., 0.,
                              image->width(),
                              image->height());
    mRelBoundingRectSk = QRectFToSkRect(mRelBoundingRect);
    updateRelBoundingRect();
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
    ImageBoxRenderData *imageData = (ImageBoxRenderData*)data;
    imageData->image = mAnimationCacheHandler->getFrameAtFrame(
                                    mUpdateAnimationFrame);;
}

BoundingBoxRenderData *AnimationBox::createRenderData() {
    return new ImageBoxRenderData();
}
