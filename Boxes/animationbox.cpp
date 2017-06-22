#include "animationbox.h"
#include "BoxesList/boxsinglewidget.h"
#include "Animators/animatorupdater.h"
#include "canvas.h"
#include "filesourcescache.h"

AnimationBox::AnimationBox(BoxesGroup *parent) :
    BoundingBox(parent, TYPE_IMAGE) {
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

void AnimationBox::updateCurrentAnimationFrameIfNeeded() {
    if(!mCurrentAnimationFrameChanged) return;
    mCurrentAnimationFrameChanged = false;
    updateCurrentAnimationFrame();
}

void AnimationBox::scheduleUpdate() {
    updateCurrentAnimationFrameIfNeeded();
    BoundingBox::scheduleUpdate();
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

    mCurrentAnimationFrame = pixId;

    if(mAnimationCacheHandler->getFrameAtFrame(
                mCurrentAnimationFrame).get() == NULL) {
        mAnimationCacheHandler->scheduleFrameLoad(mCurrentAnimationFrame);
    }
}

void AnimationBox::prp_setAbsFrame(const int &frame) {
    BoundingBox::prp_setAbsFrame(frame);
    if(mAnimationCacheHandler == NULL) return;
    mCurrentAnimationFrameChanged = true;

    scheduleSoftUpdate();
}

void AnimationBox::afterSuccessfulUpdate() {
    mRelBoundingRect = mUpdateRelBoundingRect;
    mRelBoundingRectSk = QRectFToSkRect(mRelBoundingRect);
    updateRelBoundingRect();
}

void AnimationBox::updateUpdateRelBoundingRectFromImage() {
    mUpdateRelBoundingRect = QRectF(0., 0.,
                mUpdateAnimationImageSk->width(),
                mUpdateAnimationImageSk->height());//mUpdateAnimationImage.rect();
}

void AnimationBox::setUpdateVars() {
    BoundingBox::setUpdateVars();
    mUpdateAnimationImageSk = mAnimationCacheHandler->getFrameAtFrame(
                                            mCurrentAnimationFrame);
    updateUpdateRelBoundingRectFromImage();
//    CacheContainer *cont =
//            mAnimationFramesCache.getRenderContainerAtRelFrame(
//                mUpdateAnimationFrame);

//    if(cont != NULL) {
//        mUpdateAnimationImageSk = cont->getImageSk();
//        updateUpdateRelBoundingRectFromImage();
//    }
}

void AnimationBox::drawSk(SkCanvas *canvas) {
    SkPaint paint;
    //paint.setFilterQuality(kHigh_SkFilterQuality);
    canvas->drawImage(mUpdateAnimationImageSk, 0, 0, &paint);
}

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
}
