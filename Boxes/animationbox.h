#ifndef ANIMATIONBOX_H
#define ANIMATIONBOX_H
#include "imagebox.h"
#include "Animators/intanimator.h"

class AnimationBox : public ImageBox
{
public:
    AnimationBox(BoxesGroup *parent, const QStringList &listOfFrames);
    void updateAfterFrameChanged(int currentFrame);
    void updateAnimationFrame();
    void drawKeys(QPainter *p,
                  qreal pixelsPerFrame, qreal drawY,
                  int startFrame, int endFrame);
private:
    IntAnimator mFirstFrameAnimator;
    QrealAnimator mTimeScaleAnimator;
    //IntAnimator mFrameAnimator;
    QStringList mListOfFrames;
};

#endif // ANIMATIONBOX_H
