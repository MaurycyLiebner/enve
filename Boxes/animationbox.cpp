#include "animationbox.h"
#include "BoxesList/boxsinglewidget.h"

AnimationBox::AnimationBox(BoxesGroup *parent) :
    ImageBox(parent) {
    mTimeScaleAnimator.setName("time scale");
    mTimeScaleAnimator.blockPointer();
    mTimeScaleAnimator.setValueRange(-100, 100);
    mTimeScaleAnimator.setCurrentValue(1.);
    mTimeScaleAnimator.setUpdater(new AnimationBoxFrameUpdater(this));
    mTimeScaleAnimator.blockUpdater();
    addActiveAnimator(&mTimeScaleAnimator);

    setDurationRectangle(new DurationRectangle());
    mDurationRectangle->setPossibleFrameRangeVisible();
    connect(mDurationRectangle, SIGNAL(changed()),
            this, SLOT(updateAnimationFrame()));
//    mFrameAnimator.blockPointer();
//    mFrameAnimator.setValueRange(0, listOfFrames.count() - 1);
//    mFrameAnimator.setCurrentIntValue(0);
}

void AnimationBox::setListOfFrames(const QStringList &listOfFrames) {
    mListOfFrames = listOfFrames;
    if(listOfFrames.isEmpty()) return;
    updateAnimationFrame();
}

void AnimationBox::makeDuplicate(BoundingBox *targetBox) {
    BoundingBox::makeDuplicate(targetBox);
    AnimationBox *animationBoxTarget = (AnimationBox*)targetBox;
    animationBoxTarget->setListOfFrames(mListOfFrames);
    animationBoxTarget->duplicateAnimationBoxAnimatorsFrom(
                &mTimeScaleAnimator);
}

void AnimationBox::duplicateAnimationBoxAnimatorsFrom(
        QrealAnimator *timeScaleAnimator) {
    timeScaleAnimator->makeDuplicate(&mTimeScaleAnimator);
}

DurationRectangleMovable *AnimationBox::getRectangleMovableAtPos(
                            qreal relX,
                            int minViewedFrame,
                            qreal pixelsPerFrame) {
    return mDurationRectangle->getMovableAt(relX,
                                           pixelsPerFrame,
                                           minViewedFrame);
}

BoundingBox *AnimationBox::createNewDuplicate(BoxesGroup *parent) {
    return new AnimationBox(parent);
}

void AnimationBox::updateAfterFrameChanged(int currentFrame) {
    BoundingBox::updateAfterFrameChanged(currentFrame);
    updateAnimationFrame();
}

void AnimationBox::updateAnimationFrame() {
    qreal timeScale = mTimeScaleAnimator.getCurrentValue();
    mDurationRectangle->setPossibleFrameDuration(
                qRound(qAbs(timeScale*mListOfFrames.count())));
    int pixId;
    if(timeScale > 0.) {
        pixId = (getCurrentFrameFromMainWindow() -
                mDurationRectangle->getMinPossibleFrame())/timeScale;
    } else {
        pixId = mListOfFrames.count() - 1 + (getCurrentFrameFromMainWindow() -
                mDurationRectangle->getMinPossibleFrame())/timeScale;
    }

    if(pixId < 0) {
        pixId = 0;
    } else if(pixId > mListOfFrames.count() - 1){
        pixId = mListOfFrames.count() - 1;
    }

    setFilePath(mListOfFrames.at(pixId));
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
