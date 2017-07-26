#ifndef LINKBOX_H
#define LINKBOX_H
#include "canvas.h"

class ExternalLinkBox : public BoxesGroup
{
    Q_OBJECT
public:
    ExternalLinkBox();
    void reload();

    void changeSrc();

    void setSrc(const QString &src);

    BoundingBox *createNewDuplicate() {
        return new ExternalLinkBox();
    }

    void makeDuplicate(Property *targetBox) {
        BoundingBox::makeDuplicate(targetBox);
        ((ExternalLinkBox*)targetBox)->setSrc(mSrc);
    }

private:
    QString mSrc;
};

class InternalLinkBox : public BoundingBox
{
    Q_OBJECT
public:
    InternalLinkBox(BoundingBox *linkTarget);

    void setLinkTarget(BoundingBox *linkTarget) {
        mLinkTarget = linkTarget->ref<BoundingBox>();
        scheduleUpdate();
        if(linkTarget == NULL) {
            setName("Link Empty");
            return;
        }
        setName("Link " + linkTarget->getName());
    }



    bool relPointInsidePath(const QPointF &point);
    QPointF getRelCenterPosition();
    qreal getEffectsMargin();

    BoundingBox *getLinkTarget();

    BoundingBox *createLink();

    BoundingBox *createNewDuplicate() {
        return new InternalLinkBox(mLinkTarget.data());
    }

    BoundingBoxRenderData *createRenderData();
    void setupBoundingBoxRenderDataForRelFrame(const int &relFrame,
                                               BoundingBoxRenderData *data);

    QRectF getRelBoundingRectAtRelFrame(const int &relFrame);
    void anim_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                               int *lastIdentical,
                                               const int &relFrame);
    bool prp_differencesBetweenRelFrames(const int &relFrame1,
                                         const int &relFrame2);
public slots:
    void scheduleAwaitUpdateSLOT();

protected:
    QSharedPointer<BoundingBox> mLinkTarget;
};

class InternalLinkCanvas : public BoundingBox {
    Q_OBJECT
public:
    InternalLinkCanvas(Canvas *canvas) :
        BoundingBox(TYPE_INTERNAL_LINK) {
        mClipToCanvasSize->prp_setName("clip to size");
        mClipToCanvasSize->setValue(true);
        mRastarized->prp_setName("rastarized");
        mRastarized->setValue(false);
        setLinkTarget(canvas);

        ca_addChildAnimator(mClipToCanvasSize.data());
        ca_addChildAnimator(mRastarized.data());
    }

    void setLinkTarget(Canvas *linkTarget) {
        setName(linkTarget->getName());
        mLinkTarget = linkTarget->ref<Canvas>();
    }

    void setClippedToCanvasSize(const bool &clipped);

    void makeDuplicate(Property *targetBox) {
        InternalLinkCanvas *ilcTarget = (InternalLinkCanvas*)targetBox;
        ilcTarget->setLinkTarget(mLinkTarget.data());
        ilcTarget->setClippedToCanvasSize(mClipToCanvasSize);
    }

    BoundingBox *createNewDuplicate() {
        return new InternalLinkCanvas(mLinkTarget.data());
    }

    void setupBoundingBoxRenderDataForRelFrame(
            const int &relFrame, BoundingBoxRenderData *data) {
        mLinkTarget->setupBoundingBoxRenderDataForRelFrame(relFrame, data);
        BoundingBox::setupBoundingBoxRenderDataForRelFrame(relFrame, data);
    }

    BoundingBoxRenderData *createRenderData() {
        return mLinkTarget->createRenderData();
    }

    bool prp_differencesBetweenRelFrames(const int &relFrame1,
                                         const int &relFrame2) {
        return mLinkTarget->prp_differencesBetweenRelFrames(relFrame1,
                                                            relFrame2);
    }
protected:
    QSharedPointer<Canvas> mLinkTarget;
    QSharedPointer<BoolProperty> mClipToCanvasSize =
            (new BoolProperty())->ref<BoolProperty>();
    QSharedPointer<BoolProperty> mRastarized =
            (new BoolProperty())->ref<BoolProperty>();
};

#endif // LINKBOX_H
