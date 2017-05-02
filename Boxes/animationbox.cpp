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
    mRenderCacheHandler.setDurationRectangle(mDurationRectangle);
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

    if(pixId < 0) {
        pixId = 0;
    } else if(pixId > mFramesCount - 1){
        pixId = mFramesCount - 1;
    }

    mCurrentAnimationFrame = pixId;
    auto searchCurrentFrame = mAnimationFramesCache.find(
                                            mCurrentAnimationFrame);
    if(searchCurrentFrame == mAnimationFramesCache.end()) {
        schedulePixmapReload();
    } else {
        mPixmapReloadScheduled = false;
        scheduleSoftUpdate();
    }
}

void AnimationBox::afterSuccessfulUpdate() {
    mPixmapReloadScheduled = false;
    if(mUpdatePixmapReloadScheduled) {
        auto searchLastFrame = mAnimationFramesCache.find(
                                            mUpdateAnimationFrame);
        if(searchLastFrame == mAnimationFramesCache.end()) {
            mAnimationFramesCache.insert({mUpdateAnimationFrame,
                                          mUpdateAnimationImage});
        }
    }
    mRelBoundingRect = mUpdateRelBoundingRect;
    updateRelBoundingRect();
    if(!mPivotChanged) centerPivotPosition();
}

void AnimationBox::setUpdateVars() {
    BoundingBox::setUpdateVars();
    mUpdateAnimationFrame = mCurrentAnimationFrame;
    mUpdatePixmapReloadScheduled = mPixmapReloadScheduled;
    mUpdateReplaceCache = mUpdateReplaceCache || mUpdatePixmapReloadScheduled;
    if(!mUpdatePixmapReloadScheduled) {
        auto searchCurrentFrame = mAnimationFramesCache.find(mUpdateAnimationFrame);
        if(searchCurrentFrame != mAnimationFramesCache.end()) {
            mUpdateAnimationImage = searchCurrentFrame->second;
            mUpdateRelBoundingRect = mUpdateAnimationImage.rect();
        }
    }
}

void AnimationBox::schedulePixmapReload() {
    if(mPixmapReloadScheduled) return;
    mPixmapReloadScheduled = true;
    scheduleSoftUpdate();
}

void AnimationBox::preUpdatePixmapsUpdates() {
    reloadPixmapIfNeeded();
    BoundingBox::preUpdatePixmapsUpdates();
}

void AnimationBox::reloadPixmapIfNeeded() {
    if(mPixmapReloadScheduled) {
        loadUpdatePixmap();
    }
}

void AnimationBox::draw(QPainter *p) {
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->drawImage(0, 0, mUpdateAnimationImage);
}

bool AnimationBox::relPointInsidePath(QPointF point) {
    return mRelBoundingRect.contains(point.toPoint());
}

void AnimationBox::drawSelected(QPainter *p,
                            const CanvasMode &) {
    if(isVisibleAndInVisibleDurationRect()) {
        p->save();
        drawBoundingRect(p);
        p->restore();
    }
}
