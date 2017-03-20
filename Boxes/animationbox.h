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
    void drawSelected(QPainter *p, const CanvasMode &);
    bool relPointInsidePath(QPointF point);
    void draw(QPainter *p);
    void reloadPixmap();
    void reloadPixmapIfNeeded();
    void preUpdatePixmapsUpdates();
    void schedulePixmapReload();
    void setUpdateVars();
    void afterSuccessfulUpdate();
    void updateDurationRectanglePossibleRange();
public slots:
    void updateAfterDurationRectangleChanged();
private:
    bool mPixmapReloadScheduled = false;
    bool mUpdatePixmapReloadScheduled = false;
    int mUpdateAnimationFrame = 0;
    int mCurrentAnimationFrame = 0;
    int mFramesCount = 0;
    std::unordered_map<int, QImage> mAnimationFramesCache;
    QImage mUpdateAnimationImage;
    QString mUpdateFramePath = "";

    QrealAnimator mTimeScaleAnimator;
    //IntAnimator mFrameAnimator;
    QStringList mListOfFrames;
};

#endif // ANIMATIONBOX_H
