#ifndef BRUSHEFFECT_H
#define BRUSHEFFECT_H
#include "pixmapeffect.h"
class BoundingBox;
class QrealAnimator;
typedef QSharedPointer<QrealAnimator> QrealAnimatorQSPtr;
class IntAnimator;
typedef QSharedPointer<IntAnimator> IntAnimatorQSPtr;
class BoolProperty;
typedef QSharedPointer<BoolProperty> BoolPropertyQSPtr;

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
    bool randomize;
    bool smooth;
    int randStep;
    int relFrame;
    int seed;
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

    void interpolateWith(const QPointF &startPos,
                         const QPointF &startCtrlPos,
                         const QPointF &endCtrlPos,
                         const QPointF &endPos,
                         const qreal &radius,
                         const QColor &color,
                         const qreal &weight);


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

    qreal getMargin();
    qreal getMarginAtRelFrame(const int &relFrame);
    PixmapEffectRenderData *getPixmapEffectRenderDataForRelFrame(const int &relFrame);

    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                              int *lastIdentical,
                                              const int &relFrame);

    bool prp_differencesBetweenRelFrames(const int &relFrame1,
                                         const int &relFrame2);

    void prp_setAbsFrame(const int &frame) {
        ComplexAnimator::prp_setAbsFrame(frame);
        if(mRandomize->getValue()) {
            prp_callUpdater();
        }
    }
private:
    QrealAnimatorQSPtr mMinBrushRadius;
    QrealAnimatorQSPtr mMaxBrushRadius;
    QrealAnimatorQSPtr mStrokeMaxLength;
    QrealAnimatorQSPtr mStrokeMinDirectionAngle;
    QrealAnimatorQSPtr mStrokeMaxDirectionAngle;
    QrealAnimatorQSPtr mStrokeCurvature;
    QrealAnimatorQSPtr mNumberStrokes;
    QrealAnimatorQSPtr mSmoothness;
    BoolPropertyQSPtr mRandomize;
    IntAnimatorQSPtr mRandomizeStep;
    BoolPropertyQSPtr mSmoothTransform;
    IntAnimatorQSPtr mSeed;
};

#endif // BRUSHEFFECT_H
