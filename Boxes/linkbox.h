#ifndef LINKBOX_H
#define LINKBOX_H
#include "boxesgroup.h"

class ExternalLinkBox : public BoxesGroup
{
public:
    ExternalLinkBox(QString srcFile, BoxesGroup *parent);
    void reload();

    void changeSrc();

    void setSrc(const QString &src);

private:
    QString mSrc;
};

class InternalLinkBox : public BoundingBox
{
    Q_OBJECT
public:
    InternalLinkBox(BoundingBox *linkTarget, BoxesGroup *parent);

    QPixmap renderPixProvidedTransform(
                        const QMatrix &renderTransform,
                        QPointF *drawPos);

    QPixmap getAllUglyPixmapProvidedTransform(
                        const QMatrix &allUglyTransform,
                        QRectF *allUglyBoundingRectP);

    QPixmap getPrettyPixmapProvidedTransform(
                            const QMatrix &transform,
                            QRectF *pixBoundingRectClippedToViewP);

    void drawSelected(QPainter *p, CanvasMode);
    void updateBoundingRect();
    bool relPointInsidePath(QPointF point);
    QPointF getRelCenterPosition();
    qreal getEffectsMargin();

    BoundingBox *getLinkTarget();

    BoundingBox *createLink(BoxesGroup *parent);

    BoundingBox *createSameTransformationLink(BoxesGroup *parent);
public slots:
    void scheduleAwaitUpdateSLOT();

protected:
    BoundingBox *mLinkTarget = NULL;
};

class SameTransformInternalLink : public InternalLinkBox
{
public:
    SameTransformInternalLink(BoundingBox *linkTarget,
                              BoxesGroup *parent);

    void updateCombinedTransform();

    QMatrix getRelativeTransform() const;

    virtual const QPainterPath &getRelBoundingRectPath();

    qreal getEffectsMargin();
};

class InternalLinkBoxesGroup : public BoxesGroup
{
public:
    InternalLinkBoxesGroup(BoxesGroup *linkTarget,
                           BoxesGroup *parent) : BoxesGroup(parent) {
        mLinkTarget = linkTarget;
        setType(TYPE_INTERNAL_LINK);
    }

    BoundingBox *createLink(BoxesGroup *parent) {
        return mLinkTarget->createLink(parent);
    }

    BoundingBox *createSameTransformationLink(BoxesGroup *parent) {
        return mLinkTarget->createSameTransformationLink(parent);
    }
protected:
    BoundingBox *mLinkTarget = NULL;
};

class SameTransformInternalLinkBoxesGroup : public InternalLinkBoxesGroup {
public:
    SameTransformInternalLinkBoxesGroup(BoxesGroup *linkTarget,
                                        BoxesGroup *parent);

    void updateCombinedTransform();

    QMatrix getRelativeTransform() const;

    virtual const QPainterPath &getRelBoundingRectPath();

    qreal getEffectsMargin();
};

#endif // LINKBOX_H
