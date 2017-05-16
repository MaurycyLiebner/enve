#ifndef ANIMATIONBOX_H
#define ANIMATIONBOX_H
#include "imagebox.h"
#include "Animators/intanimator.h"
#include "durationrectangle.h"

class AnimationBox : public BoundingBox
{
    Q_OBJECT
public:
    AnimationBox(BoxesGroup *parent);
    void updateAfterFrameChanged(const int &currentFrame);

    void makeDuplicate(Property *targetBox);

//    BoundingBox *createNewDuplicate(BoxesGroup *parent);
    void duplicateAnimationBoxAnimatorsFrom(QrealAnimator *timeScaleAnimator);
    bool relPointInsidePath(const QPointF &point);
    void draw(QPainter *p);
    virtual void loadUpdatePixmap() = 0;
    void preUpdatePixmapsUpdates();
    void setUpdateVars();
    void afterSuccessfulUpdate();
    void updateDurationRectangleAnimationRange();
    virtual void reloadFile() = 0;
    FixedLenAnimationRect *getAnimationDurationRect();

    bool SWT_isAnimationBox() { return true; }
public slots:
protected:
    CacheHandler mAnimationFramesCache;
    bool mUpdatePixmapReloadScheduled = false;
    int mUpdateAnimationFrame = 0;
    int mCurrentAnimationFrame = 0;
    int mFramesCount = 0;
    QImage mUpdateAnimationImage;

    QSharedPointer<QrealAnimator> mTimeScaleAnimator =
            (new QrealAnimator())->ref<QrealAnimator>();
    //IntAnimator mFrameAnimator;
};

#endif // ANIMATIONBOX_H
