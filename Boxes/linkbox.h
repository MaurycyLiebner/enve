#ifndef LINKBOX_H
#define LINKBOX_H
#include "boxesgroup.h"

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
    InternalLinkBox(BoxesGroup *parent);
    InternalLinkBox(BoundingBox *linkTarget, BoxesGroup *parent);

    void setLinkTarget(BoundingBox *linkTarget) {
        mLinkTarget = linkTarget;
        scheduleSoftUpdate();
        if(linkTarget == NULL) {
            setName("Link Empty");
            return;
        }
        setName("Link " + linkTarget->getName());
        connect(linkTarget, SIGNAL(scheduleAwaitUpdateAllLinkBoxes()),
                this, SLOT(scheduleAwaitUpdateSLOT()));
    }

    QImage getAllUglyPixmapProvidedTransform(
                        const qreal &effectsMargin,
                        const QMatrix &allUglyTransform,
                        QRectF *allUglyBoundingRectP);

    QImage getPrettyPixmapProvidedTransform(
                        const QMatrix &transform,
                        QRectF *pixBoundingRectClippedToViewP);

    void drawSelected(QPainter *p, const CanvasMode &);
    void updateRelBoundingRect();
    bool relPointInsidePath(const QPointF &point);
    QPointF getRelCenterPosition();
    qreal getEffectsMargin();

    BoundingBox *getLinkTarget();

    BoundingBox *createLink(BoxesGroup *parent);

    BoundingBox *createSameTransformationLink(BoxesGroup *parent);

    BoundingBox *createNewDuplicate(BoxesGroup *parent) {
        return new InternalLinkBox(parent);
    }

    void makeDuplicate(Property *targetBox) {
        BoundingBox::makeDuplicate(targetBox);
        InternalLinkBox *linkBox = (InternalLinkBox*)targetBox;
        linkBox->setLinkTarget(mLinkTarget);
    }
public slots:
    void scheduleAwaitUpdateSLOT();

protected:
    BoundingBox *mLinkTarget = NULL;
};

class SameTransformInternalLink : public InternalLinkBox
{
    Q_OBJECT
public:
    SameTransformInternalLink(BoxesGroup *&parent);
    SameTransformInternalLink(BoundingBox *linkTarget,
                              BoxesGroup *parent);

    void updateCombinedTransform();

    QMatrix getRelativeTransform() const;

    virtual const QPainterPath &getRelBoundingRectPath();

    qreal getEffectsMargin();

    BoundingBox *createNewDuplicate(BoxesGroup *parent) {
        return new SameTransformInternalLink(parent);
    }
};

class InternalLinkBoxesGroup : public BoxesGroup
{
    Q_OBJECT
public:
    InternalLinkBoxesGroup(BoxesGroup *parent) :
        BoxesGroup(parent) {
        setType(TYPE_INTERNAL_LINK);
    }

    InternalLinkBoxesGroup(BoxesGroup *linkTarget,
                           BoxesGroup *parent) :
        InternalLinkBoxesGroup(parent) {
        setLinkTarget(linkTarget);
    }

    virtual void setLinkTarget(BoxesGroup *linkTarget) {
        mLinkTarget = linkTarget->ref<BoxesGroup>();
    }

    BoundingBox *createLink(BoxesGroup *parent) {
        return mLinkTarget->createLink(parent);
    }

    BoundingBox *createSameTransformationLink(BoxesGroup *parent) {
        return mLinkTarget->createSameTransformationLink(parent);
    }

    BoundingBox *createNewDuplicate(BoxesGroup *parent) {
        return new InternalLinkBoxesGroup(parent);
    }

    void makeDuplicate(Property *targetBox) {
        BoxesGroup::makeDuplicate(targetBox);
        InternalLinkBoxesGroup *ilbgTarget =
                (InternalLinkBoxesGroup*)targetBox;
        ilbgTarget->setLinkTarget(mLinkTarget.data());
    }

protected:
    QSharedPointer<BoxesGroup> mLinkTarget;
};

class InternalLinkCanvas : public InternalLinkBoxesGroup {
    Q_OBJECT
public:
    InternalLinkCanvas(BoxesGroup *parent) :
        InternalLinkBoxesGroup(parent) {
    }

    InternalLinkCanvas(BoxesGroup *canvas,
                       BoxesGroup *parent) :
        InternalLinkBoxesGroup(canvas, parent) {
        updateRelBoundingRect();
        centerPivotPosition();
    }

    void setLinkTarget(BoxesGroup *linkTarget) {
        InternalLinkBoxesGroup::setLinkTarget(linkTarget);
        updateRelBoundingRect();
        centerPivotPosition();
    }

    void updateRelBoundingRect();
    void setClippedToCanvasSize(const bool &clipped);

    void draw(QPainter *p);

    void makeDuplicate(Property *targetBox) {
        InternalLinkBoxesGroup::makeDuplicate(targetBox);
        InternalLinkCanvas *ilcTarget = (InternalLinkCanvas*)targetBox;
        ilcTarget->setClippedToCanvasSize(mClipToCanvasSize);
    }

    BoundingBox *createNewDuplicate(BoxesGroup *parent) {
        return new InternalLinkCanvas(parent);
    }

protected:
    bool mClipToCanvasSize = true;
};

class SameTransformInternalLinkBoxesGroup : public InternalLinkBoxesGroup {
    Q_OBJECT
public:
    SameTransformInternalLinkBoxesGroup(BoxesGroup *parent);
    SameTransformInternalLinkBoxesGroup(BoxesGroup *linkTarget,
                                        BoxesGroup *parent);

    void updateCombinedTransform();

    QMatrix getRelativeTransform() const;

    virtual const QPainterPath &getRelBoundingRectPath();

    qreal getEffectsMargin();

    BoundingBox *createNewDuplicate(BoxesGroup *parent) {
        return new SameTransformInternalLinkBoxesGroup(parent);
    }
};

#endif // LINKBOX_H
