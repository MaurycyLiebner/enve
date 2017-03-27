#include "animationbox.h"
#include "BoxesList/boxsinglewidget.h"

AnimationBox::AnimationBox(BoxesGroup *parent) :
    BoundingBox(parent, TYPE_IMAGE) {
    setName("Animation");
    mTimeScaleAnimator.prp_setName("time scale");
    mTimeScaleAnimator.blockPointer();
    mTimeScaleAnimator.qra_setValueRange(-100, 100);
    mTimeScaleAnimator.qra_setCurrentValue(1.);
    mTimeScaleAnimator.setPrefferedValueStep(0.05);
    mTimeScaleAnimator.prp_setUpdater(new AnimationBoxFrameUpdater(this));
    mTimeScaleAnimator.prp_blockUpdater();
    ca_addChildAnimator(&mTimeScaleAnimator);

    setDurationRectangle(new DurationRectangle());
    mDurationRectangle->setPossibleFrameRangeVisible();
//    mFrameAnimator.blockPointer();
//    mFrameAnimator.setValueRange(0, listOfFrames.count() - 1);
//    mFrameAnimator.setCurrentIntValue(0);
}

void AnimationBox::updateAfterDurationRectangleChanged() {
    updateAfterFrameChanged(mCurrentAbsFrame);
}

void AnimationBox::makeDuplicate(BoundingBox *targetBox) {
    BoundingBox::makeDuplicate(targetBox);
    AnimationBox *animationBoxTarget = (AnimationBox*)targetBox;
    animationBoxTarget->duplicateAnimationBoxAnimatorsFrom(
                &mTimeScaleAnimator);
}

void AnimationBox::duplicateAnimationBoxAnimatorsFrom(
        QrealAnimator *timeScaleAnimator) {
    timeScaleAnimator->prp_makeDuplicate(&mTimeScaleAnimator);
}

DurationRectangleMovable *AnimationBox::getRectangleMovableAtPos(
                            qreal relX,
                            int minViewedFrame,
                            qreal pixelsPerFrame) {
    return mDurationRectangle->getMovableAt(relX,
                                           pixelsPerFrame,
                                           minViewedFrame);
}

//BoundingBox *AnimationBox::createNewDuplicate(BoxesGroup *parent) {
//    return new AnimationBox(parent);
//}

void AnimationBox::updateDurationRectanglePossibleRange() {
    qreal timeScale = mTimeScaleAnimator.qra_getCurrentValue();

    mDurationRectangle->setPossibleFrameDuration(
                qCeil(qAbs(timeScale*mFramesCount)));
}

void AnimationBox::updateAfterFrameChanged(int currentFrame) {
    BoundingBox::updateAfterFrameChanged(currentFrame);
    qreal timeScale = mTimeScaleAnimator.qra_getCurrentValue();

    int pixId;
    if(timeScale > 0.) {
        pixId = (mCurrentAbsFrame -
                mDurationRectangle->getMinPossibleFrame())/timeScale;
    } else {
        pixId = mFramesCount - 1 + (mCurrentAbsFrame -
                mDurationRectangle->getMinPossibleFrame())/timeScale;
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
        scheduleUpdate();
    }
}

void AnimationBox::drawKeys(QPainter *p,
                            qreal pixelsPerFrame, qreal drawY,
                            int startFrame, int endFrame) {
//    qreal timeScale = mTimeScaleAnimator.getCurrentValue();
//    int startDFrame = mDurationRectangle.getMinPossibleFrame() - startFrame;
//    int frameWidth = ceil(mListOfFrames.count()/qAbs(timeScale));
//    p->fillRect(startDFrame*pixelsPerFrame + pixelsPerFrame*0.5, drawY,
//                frameWidth*pixelsPerFrame - pixelsPerFrame,
//                BOX_HEIGHT, QColor(0, 0, 255, 125));
    mDurationRectangle->draw(p, pixelsPerFrame,
                            drawY, startFrame);
    BoundingBox::drawKeys(p, pixelsPerFrame, drawY,
                          startFrame, endFrame);
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
    scheduleUpdate();
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
    if(shouldUpdateAndDraw()) {
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->drawImage(0, 0, mUpdateAnimationImage);
    }
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
