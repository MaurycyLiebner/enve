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
    void drawSelected(QPainter *p, const CanvasMode &);
    bool relPointInsidePath(const QPointF &point);
    void draw(QPainter *p);
    virtual void loadUpdatePixmap() = 0;
    void reloadPixmapIfNeeded();
    void preUpdatePixmapsUpdates();
    void schedulePixmapReload();
    void setUpdateVars();
    void afterSuccessfulUpdate();
    void updateDurationRectangleAnimationRange();
    virtual void reloadFile() = 0;
    FixedLenAnimationRect *getAnimationDurationRect();
public slots:
protected:
    bool mPixmapReloadScheduled = false;
    bool mUpdatePixmapReloadScheduled = false;
    int mUpdateAnimationFrame = 0;
    int mCurrentAnimationFrame = 0;
    int mFramesCount = 0;
    std::unordered_map<int, QImage> mAnimationFramesCache;
    QImage mUpdateAnimationImage;

    QSharedPointer<QrealAnimator> mTimeScaleAnimator =
            (new QrealAnimator())->ref<QrealAnimator>();
    //IntAnimator mFrameAnimator;
};

#endif // ANIMATIONBOX_H
