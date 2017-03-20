#ifndef ANIMATIONBOX_H
#define ANIMATIONBOX_H
#include "imagebox.h"
#include "Animators/intanimator.h"
#include "durationrectangle.h"

class AnimationBox : public ImageBox
{
    Q_OBJECT
public:
    AnimationBox(BoxesGroup *parent);
    void updateAfterFrameChanged(int currentFrame);
    void drawKeys(QPainter *p,
                  qreal pixelsPerFrame, qreal drawY,
                  int startFrame, int endFrame);

    void setListOfFrames(const QStringList &listOfFrames);

    void makeDuplicate(BoundingBox *targetBox);

    BoundingBox *createNewDuplicate(BoxesGroup *parent);
    void duplicateAnimationBoxAnimatorsFrom(QrealAnimator *timeScaleAnimator);
    DurationRectangleMovable *getRectangleMovableAtPos(
                                    qreal relX,
                                    int minViewedFrame,
                                    qreal pixelsPerFrame);
public slots:
    void updateAnimationFrame();
    void updateAfterDurationRectangleChanged();
private:
    QrealAnimator mTimeScaleAnimator;
    //IntAnimator mFrameAnimator;
    QStringList mListOfFrames;
};

#endif // ANIMATIONBOX_H
