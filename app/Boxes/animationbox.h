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
    Q_OBJECT
public:
    AnimationBox();
    ~AnimationBox();
    void anim_setAbsFrame(const int &frame);

    void updateDurationRectangleAnimationRange();
    void reloadCacheHandler();
    virtual void reloadSound() {}
    FixedLenAnimationRect *getAnimationDurationRect();

    bool SWT_isAnimationBox() const { return true; }
    void addActionsToMenu(QMenu * const menu, QWidget* const widgetsParent);
    virtual void changeSourceFile(QWidget* dialogParent) = 0;
    void setupBoundingBoxRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData* data);
    stdsptr<BoundingBoxRenderData> createRenderData();

    void afterUpdate();
    void setParentGroup(BoxesGroup * const parent);
    void beforeAddingScheduler();
    bool shouldScheduleUpdate();
    int getAnimationFrameForRelFrame(const int &relFrame);

    void enableFrameRemapping();
    void disableFrameRemapping();
public slots:
protected:
    bool mNewCurrentFrameUpdateNeeded = false;
    stdptr<AnimationCacheHandler> mAnimationCacheHandler;
    qreal mFps = 24;

    bool mFrameRemappingEnabled = false;
    qsptr<IntAnimator> mFrameAnimator;
};

#endif // ANIMATIONBOX_H
