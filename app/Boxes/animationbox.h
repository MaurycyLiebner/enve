#ifndef ANIMATIONBOX_H
#define ANIMATIONBOX_H
#include "Animators/intanimator.h"
#include "GUI/Timeline/fixedlenanimationrect.h"
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
    friend class SelfRef;
protected:
    AnimationBox(const BoundingBoxType &type);
public:
    virtual void changeSourceFile(QWidget* dialogParent) = 0;
    virtual void reloadSound() {}
    virtual void setStretch(const qreal stretch) {
        mStretch = stretch;
        updateDurationRectangleAnimationRange();
    }

    void anim_setAbsFrame(const int frame);

    FrameRange prp_getIdenticalRelRange(const int relFrame) const;

    void reloadCacheHandler();

    bool SWT_isAnimationBox() const { return true; }
    void addActionsToMenu(BoxTypeMenu * const menu);
    void setupRenderData(const qreal relFrame,
                         BoundingBoxRenderData * const data);
    stdsptr<BoundingBoxRenderData> createRenderData();
    void setParentGroup(ContainerBox * const parent);
    bool shouldPlanScheduleUpdate();

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
    stdsptr<AnimationFrameHandler> mSrcFramesCache;

    bool mFrameRemappingEnabled = false;
    qsptr<IntAnimator> mFrameAnimator;
};

#endif // ANIMATIONBOX_H
