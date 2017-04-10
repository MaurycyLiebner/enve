#ifndef BOUNDINGBOXRENDERCONTAINER_H
#define BOUNDINGBOXRENDERCONTAINER_H
class BoundingBox;
#include <QImage>
#include "selfref.h"

class BoundingBoxRenderContainer {
public:
    BoundingBoxRenderContainer() {}
    virtual ~BoundingBoxRenderContainer() {}

    void draw(QPainter *p);

    void updatePaintTransformGivenNewCombinedTransform(
            const QMatrix &combinedTransform);

    void replaceImage(const QImage &img);

    void setTransform(const QMatrix &transform);

    void setBoundingRect(const QRectF &rect);

    const QMatrix &getTransform() const;

    const QImage &getImage() const;

    const QMatrix &getPaintTransform() const;

    const QRectF &getBoundingRect() const;

    const qreal &getResolutionPercent() const;

    const int &getFrame() const;

    const qint64 &getRenderTime() const;

    void setRelFrame(const int &frame);

    void updateVariables(const QMatrix &combinedTransform,
                         const qreal &effectsMargin,
                         const qreal &resolutionPer,
                         BoundingBox *target);

    void drawWithoutTransform(QPainter *p);

    int getByteCount() {
        return mImage.byteCount();
    }

protected:
    qint64 mRenderTime = 0;
    int mFrame = 0;
    qreal mResolutionPercent;
    QMatrix mTransform;
    QMatrix mPaintTransform;
    QRectF mBoundingRect;
    QImage mImage;
};

class RenderCacheRange;

class CacheBoundingBoxRenderContainer : public BoundingBoxRenderContainer,
                                        public SimpleSmartPointer {
public:
    CacheBoundingBoxRenderContainer();
    virtual ~CacheBoundingBoxRenderContainer();

    void duplicateFrom(BoundingBoxRenderContainer *src);

    void setVariables(const QMatrix &transform,
                      const QMatrix &paintTransform,
                      const QRectF &rect,
                      const QImage &img,
                      const int &frame,
                      const qreal &res, const qint64 &time);
    void freeThis();

    void setParentRagne(RenderCacheRange *range) {
        mParentRange = range;
    }

    void thisAccessed();
private:
    RenderCacheRange *mParentRange = NULL;
};

#endif // BOUNDINGBOXRENDERCONTAINER_H
