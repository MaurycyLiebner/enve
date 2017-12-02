#ifndef BRUSHEFFECT_H
#define BRUSHEFFECT_H
#include "pixmapeffect.h"
class BoundingBox;
class QrealAnimator;
typedef QSharedPointer<QrealAnimator> QrealAnimatorQSPtr;

struct BrushEffectRenderData : public PixmapEffectRenderData {
    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);
    qreal minBrushRadius;
    qreal maxBrushRadius;
    qreal strokeMaxLength;
    qreal strokeMinDirectionAngle;
    qreal strokeMaxDirectionAngle;
    qreal strokeCurvature;
    qreal numberStrokes;
};
class Dab;
class Brush;
class BrushStroke {
public:
    BrushStroke(const QPointF &startPos,
                const QPointF &startCtrlPos,
                const QPointF &endCtrlPos,
                const QPointF &endPos,
                const qreal &radius,
                const QColor &color);


    void prepareToDrawOnImage(const SkBitmap &img);
    void drawOnImage(unsigned char *data,
                     const int &width, const int &height);
private:
    Brush *mBrush = NULL;
    QList<Dab> mDabs;

    QPainterPath mStrokePath;
    QPainterPath mWholeStrokePath;
    QRectF mBoundingRect;

    QPointF mStartPos;
    QPointF mStartCtrlPos;
    QPointF mEndCtrlPos;
    QPointF mEndPos;
    qreal mRadius;
    QColor mColor;
};

class BrushEffect : public PixmapEffect {
public:
    BrushEffect(qreal numberStrokes = 100.,
                qreal brushMinRadius = 20.,
                qreal brushMaxRadius = 30.,
                qreal strokeMaxLength = 200.,
                qreal strokeMinDirectionAngle = 0.,
                qreal strokeMaxDirectionAngle = 360.,
                qreal strokeCurvature = 0.5);

    void apply(BoundingBox *target,
               QImage *imgPtr,
               const fmt_filters::image &img,
               qreal scale);

    void makeDuplicate(Property *) {}
    Property *makeDuplicate() { return NULL; }

    qreal getMargin();
    qreal getMarginAtRelFrame(const int &relFrame);
    PixmapEffectRenderData *getPixmapEffectRenderDataForRelFrame(const int &relFrame);
private:
    QrealAnimatorQSPtr mMinBrushRadius;
    QrealAnimatorQSPtr mMaxBrushRadius;
    QrealAnimatorQSPtr mStrokeMaxLength;
    QrealAnimatorQSPtr mStrokeMinDirectionAngle;
    QrealAnimatorQSPtr mStrokeMaxDirectionAngle;
    QrealAnimatorQSPtr mStrokeCurvature;
    QrealAnimatorQSPtr mNumberStrokes;
};

#endif // BRUSHEFFECT_H
