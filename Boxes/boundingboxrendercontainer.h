#ifndef BOUNDINGBOXRENDERCONTAINER_H
#define BOUNDINGBOXRENDERCONTAINER_H
class BoundingBox;
#include <QImage>

class BoundingBoxRenderContainer {
public:
    BoundingBoxRenderContainer();

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

    void setFrame(const int &frame);

    void updateVariables(const QMatrix &combinedTransform,
                         const qreal &effectsMargin,
                         const qreal &resolutionPer,
                         BoundingBox *target);

    void duplicateFrom(BoundingBoxRenderContainer *src);

    void setVariables(const QMatrix &transform,
                      const QMatrix &paintTransform,
                      const QRectF &rect,
                      const QImage &img,
                      const int &frame, const qreal &res);
    void drawWithoutTransform(QPainter *p);
private:
    int mFrame = 0;
    qreal mResolutionPercent;
    QMatrix mTransform;
    QMatrix mPaintTransform;
    QRectF mBoundingRect;
    QImage mImage;
};

#endif // BOUNDINGBOXRENDERCONTAINER_H
