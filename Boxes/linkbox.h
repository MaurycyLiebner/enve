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
        if(mLinkTarget != NULL) {
            disconnect(mLinkTarget.data(), SIGNAL(scheduledUpdate()),
                       this, SLOT(scheduleUpdate()));
        }
        if(linkTarget == NULL) {
            setName("empty link");

            mLinkTarget.reset();
        } else {
            setName(linkTarget->getName() + " link");
            mLinkTarget = linkTarget->ref<BoundingBox>();
            connect(linkTarget, SIGNAL(scheduledUpdate()),
                    this, SLOT(scheduleUpdate()));
        }
        scheduleUpdate();
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
    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                              int *lastIdentical,
                                              const int &relFrame);
    bool prp_differencesBetweenRelFrames(const int &relFrame1,
                                         const int &relFrame2);
    void addSchedulersToProcess();
public slots:
    void scheduleAwaitUpdateSLOT();

protected:
    QSharedPointer<BoundingBox> mLinkTarget;
};

class InternalLinkCanvas : public InternalLinkBox {
    Q_OBJECT
public:
    InternalLinkCanvas(Canvas *canvas) :
        InternalLinkBox(canvas) {
        mType = TYPE_INTERNAL_LINK_CANVAS;
        mClipToCanvasSize->prp_setName("clip to size");
        mClipToCanvasSize->setValue(true);
        mRastarized->prp_setName("rastarized");
        mRastarized->setValue(false);

        ca_addChildAnimator(mClipToCanvasSize.data());
        ca_addChildAnimator(mRastarized.data());
    }

    void prp_setAbsFrame(const int &frame) {
        BoundingBox::prp_setAbsFrame(frame);
        ((Canvas*)mLinkTarget.data())->prp_setAbsFrame(
                    anim_mCurrentRelFrame);
    }

    void setClippedToCanvasSize(const bool &clipped);

    void makeDuplicate(Property *targetBox) {
        InternalLinkCanvas *ilcTarget = (InternalLinkCanvas*)targetBox;
        ilcTarget->setLinkTarget(mLinkTarget.data());
        ilcTarget->setClippedToCanvasSize(mClipToCanvasSize);
    }

    BoundingBox *createNewDuplicate() {
        return new InternalLinkCanvas((Canvas*)mLinkTarget.data());
    }

    void setupBoundingBoxRenderDataForRelFrame(
            const int &relFrame, BoundingBoxRenderData *data) {
        ((Canvas*)mLinkTarget.data())->BoxesGroup::
                setupBoundingBoxRenderDataForRelFrame(relFrame, data);
        BoundingBox::setupBoundingBoxRenderDataForRelFrame(relFrame, data);
        data->transform = QMatrix();
    }

    BoundingBoxRenderData *createRenderData() {
        BoundingBoxRenderData *renderData =
                ((Canvas*)mLinkTarget.data())->BoxesGroup::createRenderData();
        renderData->parentBox = weakRef<BoundingBox>();
        return renderData;
    }

    bool prp_differencesBetweenRelFrames(const int &relFrame1,
                                         const int &relFrame2) {
        return mLinkTarget->prp_differencesBetweenRelFrames(relFrame1,
                                                            relFrame2);
    }

    QRectF getRelBoundingRectAtRelFrame(const int &relFrame);
protected:
    QSharedPointer<BoolProperty> mClipToCanvasSize =
            (new BoolProperty())->ref<BoolProperty>();
    QSharedPointer<BoolProperty> mRastarized =
            (new BoolProperty())->ref<BoolProperty>();
};

#endif // LINKBOX_H
