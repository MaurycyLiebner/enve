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
    InternalLinkBox(BoundingBox *linkTarget, BoxesGroup *parent) :
        BoundingBox(parent, TYPE_LINK) {
        mLinkTarget = linkTarget;
        connect(linkTarget, SIGNAL(scheduleAwaitUpdateAllLinkBoxes()),
                this, SLOT(scheduleAwaitUpdateSLOT()));
    }

    QPixmap renderPixProvidedTransform(
                        const QMatrix &renderTransform,
                        QPointF *drawPos) {
        return mLinkTarget->renderPixProvidedTransform(renderTransform,
                                                       drawPos);
    }

    QPixmap getAllUglyPixmapProvidedTransform(
                        const QMatrix &allUglyTransform,
                        QRectF *allUglyBoundingRectP) {
        return mLinkTarget->getAllUglyPixmapProvidedTransform(allUglyTransform,
                                                       allUglyBoundingRectP);
    }

    QPixmap getPrettyPixmapProvidedTransform(
                            const QMatrix &transform,
                            QRectF *pixBoundingRectClippedToViewP) {
        return mLinkTarget->getPrettyPixmapProvidedTransform(transform,
                                                pixBoundingRectClippedToViewP);
    }

    void drawSelected(QPainter *p, CanvasMode);
    void updateBoundingRect();
    bool relPointInsidePath(QPointF point);
    QPointF getRelCenterPosition();
    qreal getEffectsMargin();
public slots:
    void scheduleAwaitUpdateSLOT() {
        scheduleAwaitUpdate();
    }

private:
    BoundingBox *mLinkTarget = NULL;
};

#endif // LINKBOX_H
