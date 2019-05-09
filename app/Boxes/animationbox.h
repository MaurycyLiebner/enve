#ifndef ANIMATIONBOX_H
#define ANIMATIONBOX_H
#include "Animators/intanimator.h"
#include "durationrectangle.h"
#include "boundingbox.h"
#include "rendercachehandler.h"
#include "imagebox.h"
class AnimationCacheHandler;

struct AnimationBoxRenderData : public ImageBoxRenderData {
    AnimationBoxRenderData(FileCacheHandler *cacheHandler,
                           BoundingBox *parentBox) :
        ImageBoxRenderData(cacheHandler, parentBox) {}

    void loadImageFromHandler();
    int fAnimationFrame;
};

class AnimationBox : public BoundingBox {
    friend class SelfRef;
protected:
    AnimationBox(const BoundingBoxType &type);
public:
    ~AnimationBox();
    void anim_setAbsFrame(const int &frame);

    FrameRange prp_getIdenticalRelRange(const int &relFrame) const;

    void reloadCacheHandler();
    virtual void reloadSound() {}

    bool SWT_isAnimationBox() const { return true; }
    void addActionsToMenu(BoxTypeMenu * const menu);
    virtual void changeSourceFile(QWidget* dialogParent) = 0;
    void setupRenderData(const qreal &relFrame,
                         BoundingBoxRenderData * const data);
    stdsptr<BoundingBoxRenderData> createRenderData();
    void setParentGroup(BoxesGroup * const parent);
    bool shouldPlanScheduleUpdate();

    void writeBoundingBox(QIODevice * const target);
    void readBoundingBox(QIODevice * const target);

    FixedLenAnimationRect *getAnimationDurationRect() const;
    void updateDurationRectangleAnimationRange();

    void afterUpdate();
    void beforeAddingScheduler();
    int getAnimationFrameForRelFrame(const int &relFrame);

    void enableFrameRemapping();
    void disableFrameRemapping();

    void reload();
protected:
    bool mNewCurrentFrameUpdateNeeded = false;
    stdptr<AnimationCacheHandler> mSrcFramesCache;

    bool mFrameRemappingEnabled = false;
    qsptr<IntAnimator> mFrameAnimator;
};

#endif // ANIMATIONBOX_H
