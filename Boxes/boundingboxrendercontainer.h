#ifndef BOUNDINGBOXRENDERCONTAINER_H
#define BOUNDINGBOXRENDERCONTAINER_H
class BoundingBox;
#include <QImage>
#include "selfref.h"

class RenderCacheHandler;

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

    const qreal &getResolutionFraction() const;

    const int &getMinRelFrame() const;

    const int &getMaxRelFrame() const;

    const qint64 &getRenderTime() const;

    void setRelFrame(const int &frame);
    void setMaxRelFrame(const int &maxFrame);
    void setMinRelFrame(const int &minFrame);
    void setRelFrameRange(const int &minFrame, const int &maxFrame);

    void updateVariables(const QMatrix &combinedTransform,
                         const qreal &effectsMargin,
                         const qreal &resolutionPer,
                         BoundingBox *target);

    void drawWithoutTransform(QPainter *p);

    int getByteCount() {
        return mImage.byteCount();
    }

    bool relFrameInRange(const int &relFrame);
    void setVariables(const QMatrix &transform,
                      const QMatrix &paintTransform,
                      const QRectF &rect,
                      const QImage &img,
                      const int &minFrame,
                      const int &maxFrame,
                      const qreal &res,
                      const qint64 &time);
    void duplicateFrom(BoundingBoxRenderContainer *src);
protected:
    qint64 mRenderTime = 0;
    int mMinRelFrame = 0;
    int mMaxRelFrame = 0;
    qreal mResolutionFraction;
    QMatrix mTransform;
    QMatrix mPaintTransform;
    QRectF mBoundingRect;
    QImage mImage;
};

class CacheBoundingBoxRenderContainer : public BoundingBoxRenderContainer,
                                        public SimpleSmartPointer {
public:
    CacheBoundingBoxRenderContainer();
    virtual ~CacheBoundingBoxRenderContainer();

    void setVariables(const QMatrix &transform,
                      const QMatrix &paintTransform,
                      const QRectF &rect,
                      const QImage &img,
                      const int &minFrame,
                      const int &maxFrame,
                      const qreal &res,
                      const qint64 &time);
    void setParentCacheHandler(RenderCacheHandler *handler) {
        mParentCacheHandler = handler;
    }
    void freeThis();


    void thisAccessed();
private:
    RenderCacheHandler *mParentCacheHandler = NULL;

};

#endif // BOUNDINGBOXRENDERCONTAINER_H
