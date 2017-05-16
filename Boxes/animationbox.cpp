#include "animationbox.h"
#include "BoxesList/boxsinglewidget.h"

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
    qreal timeScale = mTimeScaleAnimator->qra_getCurrentValue();

    getAnimationDurationRect()->setAnimationFrameDuration(
                qCeil(qAbs(timeScale*mFramesCount)));
}

void AnimationBox::updateAfterFrameChanged(const int &currentFrame) {
    BoundingBox::updateAfterFrameChanged(currentFrame);
    qreal timeScale = mTimeScaleAnimator->qra_getCurrentValue();

    int pixId;
    const int &absMinAnimation =
                getAnimationDurationRect()->getMinAnimationFrameAsAbsFrame();
    if(timeScale > 0.) {
        pixId = (anim_mCurrentAbsFrame - absMinAnimation)/timeScale;
    } else {
        pixId = mFramesCount - 1 +
                (anim_mCurrentAbsFrame - absMinAnimation)/timeScale;
    }

    if(pixId <= 0) {
        pixId = 0;
    } else if(pixId > mFramesCount - 1){
        pixId = mFramesCount - 1;
    }

    mCurrentAnimationFrame = pixId;

    scheduleSoftUpdate();
}

void AnimationBox::afterSuccessfulUpdate() {
    if(mUpdatePixmapReloadScheduled) {
        CacheContainer *cont = mAnimationFramesCache.getRenderContainerAtRelFrame(
                                    mUpdateAnimationFrame);
        if(cont == NULL) {
            cont = mAnimationFramesCache.createNewRenderContainerAtRelFrame(
                                                        mUpdateAnimationFrame);
            cont->setImage(mUpdateAnimationImage);
        }
    }
    mRelBoundingRect = mUpdateRelBoundingRect;
    updateRelBoundingRect();
    if(!mPivotChanged) centerPivotPosition();
}

void AnimationBox::setUpdateVars() {
    BoundingBox::setUpdateVars();
    mUpdateAnimationFrame = mCurrentAnimationFrame;
    CacheContainer *cont =
            mAnimationFramesCache.getRenderContainerAtRelFrame(
                mUpdateAnimationFrame);
    mUpdatePixmapReloadScheduled = cont == NULL;
    if(cont != NULL) {
        mUpdateAnimationImage = cont->getImage();
        mUpdateRelBoundingRect = mUpdateAnimationImage.rect();
    }
}

void AnimationBox::preUpdatePixmapsUpdates() {
    if(mUpdatePixmapReloadScheduled) {
        loadUpdatePixmap();
    }
    BoundingBox::preUpdatePixmapsUpdates();
}

void AnimationBox::draw(QPainter *p) {
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->drawImage(0, 0, mUpdateAnimationImage);
}

bool AnimationBox::relPointInsidePath(const QPointF &point) {
    return mRelBoundingRect.contains(point.toPoint());
}
