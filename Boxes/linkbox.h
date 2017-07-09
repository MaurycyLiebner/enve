#ifndef LINKBOX_H
#define LINKBOX_H
#include "canvas.h"

class ExternalLinkBox : public BoxesGroup
{
    Q_OBJECT
public:
    ExternalLinkBox(BoxesGroup *parent);
    void reload();

    void changeSrc();

    void setSrc(const QString &src);

    BoundingBox *createNewDuplicate(BoxesGroup *parent) {
        return new ExternalLinkBox(parent);
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
    InternalLinkBox(BoundingBox *linkTarget, BoxesGroup *parent);

    void setLinkTarget(BoundingBox *linkTarget) {
        mLinkTarget = linkTarget;
        scheduleSoftUpdate();
        if(linkTarget == NULL) {
            setName("Link Empty");
            return;
        }
        setName("Link " + linkTarget->getName());
    }



    void updateRelBoundingRect();
    bool relPointInsidePath(const QPointF &point);
    QPointF getRelCenterPosition();
    qreal getEffectsMargin();

    BoundingBox *getLinkTarget();

    BoundingBox *createLink(BoxesGroup *parent);

    BoundingBox *createNewDuplicate(BoxesGroup *parent) {
        return new InternalLinkBox(mLinkTarget, parent);
    }

    BoundingBoxRenderData *createRenderData();
    void setupBoundingBoxRenderDataForRelFrame(const int &relFrame,
                                               BoundingBoxRenderData *data);
public slots:
    void scheduleAwaitUpdateSLOT();

protected:
    BoundingBox *mLinkTarget = NULL;
};

class InternalLinkCanvas : public BoundingBox {
    Q_OBJECT
public:
    InternalLinkCanvas(Canvas *canvas,
                       BoxesGroup *parent) :
        BoundingBox(parent, TYPE_INTERNAL_LINK) {
        setLinkTarget(canvas);
        updateRelBoundingRect();
        centerPivotPosition();
    }

    void setLinkTarget(Canvas *linkTarget) {
        mLinkTarget = linkTarget->ref<Canvas>();
        updateRelBoundingRect();
        centerPivotPosition();
    }

    void updateRelBoundingRect();
    void setClippedToCanvasSize(const bool &clipped);

    void makeDuplicate(Property *targetBox) {
        InternalLinkCanvas *ilcTarget = (InternalLinkCanvas*)targetBox;
        ilcTarget->setLinkTarget(mLinkTarget.data());
        ilcTarget->setClippedToCanvasSize(mClipToCanvasSize);
    }

    BoundingBox *createNewDuplicate(BoxesGroup *parent) {
        return new InternalLinkCanvas(mLinkTarget.data(),
                                      parent);
    }

    void setupBoundingBoxRenderDataForRelFrame(
            const int &relFrame, BoundingBoxRenderData *data) {
        mLinkTarget->setupBoundingBoxRenderDataForRelFrame(relFrame, data);
        BoundingBox::setupBoundingBoxRenderDataForRelFrame(relFrame, data);
    }

    BoundingBoxRenderData *createRenderData() {
        return mLinkTarget->createRenderData();
    }
protected:
    QSharedPointer<Canvas> mLinkTarget;
    bool mClipToCanvasSize = true;
};

#endif // LINKBOX_H
