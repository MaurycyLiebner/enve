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
        ImageBoxRenderData(cacheHandler, parentBox) {

    }

    void loadImageFromHandler();
    int animationFrame;
};

class AnimationBox : public BoundingBox {
    friend class SelfRef;
protected:
    AnimationBox();
public:
    ~AnimationBox();
    void anim_setAbsFrame(const int &frame);

    void reloadCacheHandler();
    virtual void reloadSound() {}

    bool SWT_isAnimationBox() const { return true; }
    void addActionsToMenu(QMenu * const menu, QWidget* const widgetsParent);
    virtual void changeSourceFile(QWidget* dialogParent) = 0;
    void setupBoundingBoxRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData* data);
    stdsptr<BoundingBoxRenderData> createRenderData();
    void setParentGroup(BoxesGroup * const parent);
    bool shouldScheduleUpdate();

    FixedLenAnimationRect *getAnimationDurationRect();
    void updateDurationRectangleAnimationRange();

    void afterUpdate();
    void beforeAddingScheduler();
    int getAnimationFrameForRelFrame(const int &relFrame);

    void enableFrameRemapping();
    void disableFrameRemapping();
protected:
    bool mNewCurrentFrameUpdateNeeded = false;
    stdptr<AnimationCacheHandler> mSrcFramesCache;
    qreal mFps = 24;

    bool mFrameRemappingEnabled = false;
    qsptr<IntAnimator> mFrameAnimator;
};

#endif // ANIMATIONBOX_H
