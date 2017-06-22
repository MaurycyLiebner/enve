#ifndef ANIMATIONBOX_H
#define ANIMATIONBOX_H
#include "Animators/intanimator.h"
#include "durationrectangle.h"
#include "boundingbox.h"
#include "rendercachehandler.h"
class AnimationCacheHandler;

class AnimationBox : public BoundingBox
{
    Q_OBJECT
public:
    AnimationBox(BoxesGroup *parent);
    void prp_setAbsFrame(const int &frame);

    void makeDuplicate(Property *targetBox);

//    BoundingBox *createNewDuplicate(BoxesGroup *parent);
    void duplicateAnimationBoxAnimatorsFrom(QrealAnimator *timeScaleAnimator);
    void drawSk(SkCanvas *canvas);
    void setUpdateVars();
    void afterSuccessfulUpdate();
    void updateDurationRectangleAnimationRange();
    virtual void reloadFile() = 0;
    FixedLenAnimationRect *getAnimationDurationRect();

    bool SWT_isAnimationBox() { return true; }
    void updateUpdateRelBoundingRectFromImage();
    void addActionsToMenu(QMenu *menu);
    bool handleSelectedCanvasAction(QAction *selectedAction);
    virtual void changeSourceFile() = 0;
    void setupBoundingBoxRenderDataForRelFrame(
                                const int &relFrame,
                                BoundingBoxRenderData *data);


    void updateCurrentAnimationFrame();
    void updateCurrentAnimationFrameIfNeeded();
    void scheduleUpdate();
public slots:
protected:
    bool mCurrentAnimationFrameChanged = false;
    AnimationCacheHandler *mAnimationCacheHandler = NULL;
    int mCurrentAnimationFrame = 0;
    qreal mFps = 24.;
    sk_sp<SkImage> mUpdateAnimationImageSk;

    QSharedPointer<QrealAnimator> mTimeScaleAnimator =
            (new QrealAnimator())->ref<QrealAnimator>();
    //IntAnimator mFrameAnimator;
};

#endif // ANIMATIONBOX_H
