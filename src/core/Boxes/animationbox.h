#ifndef ANIMATIONBOX_H
#define ANIMATIONBOX_H
#include "Animators/intanimator.h"
#include "Timeline/fixedlenanimationrect.h"
#include "boundingbox.h"
#include "imagebox.h"
class AnimationFrameHandler;

struct AnimationBoxRenderData : public ImageRenderData {
    AnimationBoxRenderData(AnimationFrameHandler *cacheHandler,
                           BoundingBox *parentBox) :
        ImageRenderData(parentBox) {
        fSrcCacheHandler = cacheHandler;
    }

    void loadImageFromHandler();

    AnimationFrameHandler *fSrcCacheHandler;
    int fAnimationFrame;
};

class AnimationBox : public BoundingBox {
    e_OBJECT
protected:
    AnimationBox(const eBoxType type);
public:
    virtual void changeSourceFile(QWidget * const dialogParent) = 0;
    virtual void animationDataChanged();
    virtual void setStretch(const qreal stretch) {
        mStretch = stretch;
        updateDurationRectangleAnimationRange();
    }

    void anim_setAbsFrame(const int frame);

    FrameRange prp_getIdenticalRelRange(const int relFrame) const;


    bool SWT_isAnimationBox() const { return true; }
    void setupCanvasMenu(PropertyMenu * const menu);
    void setupRenderData(const qreal relFrame,
                         BoxRenderData * const data);
    stdsptr<BoxRenderData> createRenderData();
    bool shouldScheduleUpdate();

    void writeBoundingBox(QIODevice * const target);
    void readBoundingBox(QIODevice * const target);

    FixedLenAnimationRect *getAnimationDurationRect() const;
    void updateDurationRectangleAnimationRange();

    void afterUpdate();
    void beforeAddingScheduler();
    int getAnimationFrameForRelFrame(const qreal relFrame);

    void enableFrameRemappingAction();
    void enableFrameRemapping();
    void disableFrameRemapping();

    qreal getStretch() const { return mStretch; }

    void reload();
protected:
    qreal mStretch = 1;

    bool mNewCurrentFrameUpdateNeeded = false;
    qsptr<AnimationFrameHandler> mSrcFramesCache;

    bool mFrameRemappingEnabled = false;
    qsptr<IntAnimator> mFrameAnimator;
};

#endif // ANIMATIONBOX_H
