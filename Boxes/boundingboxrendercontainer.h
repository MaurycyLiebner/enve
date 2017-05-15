#ifndef BOUNDINGBOXRENDERCONTAINER_H
#define BOUNDINGBOXRENDERCONTAINER_H
class BoundingBox;
#include <QImage>
#include "selfref.h"

class CacheHandler;

class CacheContainer : public SimpleSmartPointer {
public:
    CacheContainer();

    virtual ~CacheContainer();

    void setParentCacheHandler(CacheHandler *handler);
    void freeThis();

    void thisAccessed();

    virtual int getByteCount() = 0;

    const int &getMinRelFrame() const;

    const int &getMaxRelFrame() const;

    void setRelFrame(const int &frame);
    void setMaxRelFrame(const int &maxFrame);
    void setMinRelFrame(const int &minFrame);
    void setRelFrameRange(const int &minFrame, const int &maxFrame);
    bool relFrameInRange(const int &relFrame);
private:
    CacheHandler *mParentCacheHandler = NULL;
    int mMinRelFrame = 0;
    int mMaxRelFrame = 0;
};

class RenderContainer : public CacheContainer {
public:
    RenderContainer() {}
    virtual ~RenderContainer() {}

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

    void updateVariables(const QMatrix &combinedTransform,
                         const qreal &effectsMargin,
                         const qreal &resolutionPer,
                         BoundingBox *target);

    void drawWithoutTransform(QPainter *p);

    void setVariables(const QMatrix &transform,
                      const QMatrix &paintTransform,
                      const QRectF &rect,
                      const QImage &img,
                      const qreal &res);
    void duplicateFrom(RenderContainer *src);

    int getByteCount() {
        return mImage.byteCount();
    }
protected:
    qreal mResolutionFraction;
    QMatrix mTransform;
    QMatrix mPaintTransform;
    QRectF mBoundingRect;
    QImage mImage;
};

#endif // BOUNDINGBOXRENDERCONTAINER_H
