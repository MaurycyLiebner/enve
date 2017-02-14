#include "animationbox.h"
#include "BoxesList/boxsinglewidget.h"

AnimationBox::AnimationBox(BoxesGroup *parent,
                           const QStringList &listOfFrames) :
    ImageBox(parent) {
    mListOfFrames = listOfFrames;
    mFirstFrameAnimator.setName("start frame");
    mFirstFrameAnimator.blockPointer();
    mFirstFrameAnimator.setCurrentIntValue(getCurrentFrame());
    mFirstFrameAnimator.setUpdater(new AnimationBoxFrameUpdater(this));
    addActiveAnimator(&mFirstFrameAnimator);

    mTimeScaleAnimator.setName("time scale");
    mTimeScaleAnimator.blockPointer();
    mTimeScaleAnimator.setValueRange(-100, 100);
    mTimeScaleAnimator.setCurrentValue(1.);
    mTimeScaleAnimator.setUpdater(new AnimationBoxFrameUpdater(this));
    addActiveAnimator(&mTimeScaleAnimator);
//    mFrameAnimator.blockPointer();
//    mFrameAnimator.setValueRange(0, listOfFrames.count() - 1);
//    mFrameAnimator.setCurrentIntValue(0);
    setFilePath(listOfFrames.first());
}

void AnimationBox::updateAfterFrameChanged(int currentFrame) {
    //mFrameAnimator.setFrame(currentFrame);

    mFirstFrameAnimator.setFrame(currentFrame);
    updateAnimationFrame();
    //setFilePath(mListOfFrames.at(mFrameAnimator.getCurrentIntValue()));
}

void AnimationBox::updateAnimationFrame() {
    qreal timeScale = mTimeScaleAnimator.getCurrentValue();
    int pixId;
    if(timeScale > 0.) {
        pixId = (getCurrentFrame() -
                mFirstFrameAnimator.getCurrentIntValue())*timeScale;
    } else {
        pixId = mListOfFrames.count() - 1 + (getCurrentFrame() -
                mFirstFrameAnimator.getCurrentIntValue())*timeScale;
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
    qreal timeScale = mTimeScaleAnimator.getCurrentValue();
    int startDFrame = mFirstFrameAnimator.getCurrentIntValue() - startFrame;
    int frameWidth = ceil(mListOfFrames.count()/qAbs(timeScale));
    p->fillRect(startDFrame*pixelsPerFrame + pixelsPerFrame*0.5, drawY,
                frameWidth*pixelsPerFrame - pixelsPerFrame,
                BOX_HEIGHT, QColor(0, 0, 255, 125));
    BoundingBox::drawKeys(p, pixelsPerFrame, drawY,
                          startFrame, endFrame);
}
