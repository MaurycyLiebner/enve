#ifndef ANIMATIONBOX_H
#define ANIMATIONBOX_H
#include "imagebox.h"
#include "Animators/intanimator.h"

class AnimationBox : public ImageBox
{
public:
    AnimationBox(BoxesGroup *parent);
    void updateAfterFrameChanged(int currentFrame);
    void updateAnimationFrame();
    void drawKeys(QPainter *p,
                  qreal pixelsPerFrame, qreal drawY,
                  int startFrame, int endFrame);

    void setListOfFrames(const QStringList &listOfFrames);

    void makeDuplicate(BoundingBox *targetBox);

    BoundingBox *createNewDuplicate(BoxesGroup *parent);
    void duplicateAnimationBoxAnimatorsFrom(
            IntAnimator *firstFrameAnimator,
            QrealAnimator *timeScaleAnimator);
private:
    IntAnimator mFirstFrameAnimator;
    QrealAnimator mTimeScaleAnimator;
    //IntAnimator mFrameAnimator;
    QStringList mListOfFrames;
};

#endif // ANIMATIONBOX_H
