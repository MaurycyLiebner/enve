#ifndef ANIMATIONBOX_H
#define ANIMATIONBOX_H
#include "Animators/intanimator.h"
#include "durationrectangle.h"
#include "boundingbox.h"
#include "rendercachehandler.h"
#include "imagebox.h"
class AnimationCacheHandler;

class AnimationBox : public BoundingBox {
    Q_OBJECT
public:
    AnimationBox();
    void prp_setAbsFrame(const int &frame);

    void makeDuplicate(Property *targetBox);

//    BoundingBox *createNewDuplicate(BoxesGroup *parent);
    void duplicateAnimationBoxAnimatorsFrom(QrealAnimator *timeScaleAnimator);
    void updateDurationRectangleAnimationRange();
    virtual void reloadFile();
    virtual void reloadSound() {}
    FixedLenAnimationRect *getAnimationDurationRect();

    bool SWT_isAnimationBox() { return true; }
    void addActionsToMenu(QMenu *menu);
    bool handleSelectedCanvasAction(QAction *selectedAction);
    virtual void changeSourceFile() = 0;
    void setupBoundingBoxRenderDataForRelFrame(
                                const int &relFrame,
                                BoundingBoxRenderData *data);

    BoundingBoxRenderData *createRenderData();


    void updateCurrentAnimationFrame();

    void afterUpdate();
    void setParent(BoxesGroup *parent);
    void beforeAddingScheduler();
    bool shouldScheduleUpdate();
public slots:
protected:
    bool mNewCurrentFrameUpdateNeeded = false;
    AnimationCacheHandler *mAnimationCacheHandler = NULL;
    int mCurrentAnimationFrame = -1;
    int mUpdateAnimationFrame = 0;
    qreal mFps = 24.;

    QSharedPointer<QrealAnimator> mTimeScaleAnimator =
            (new QrealAnimator())->ref<QrealAnimator>();
    //IntAnimator mFrameAnimator;
};

#endif // ANIMATIONBOX_H
