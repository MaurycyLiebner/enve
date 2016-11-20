#ifndef PIXMAPEFFECT_H
#define PIXMAPEFFECT_H
#include "fmt_filters.h"
#include "complexanimator.h"

class PixmapEffect : public ComplexAnimator
{
public:
    PixmapEffect();
    virtual void apply(QImage *imgPtr,
                       const fmt_filters::image &img, qreal scale = 1.) {}
    virtual qreal getMargin() {}
};

class BlurEffect : public PixmapEffect
{
public:
    BlurEffect(qreal radius = 0.);

    void apply(QImage *imgPtr, const fmt_filters::image &img, qreal scale = 1.);

    qreal getMargin();
private:
    QrealAnimator mBlurRadius;
};

class BrushStroke {
public:
    BrushStroke(QPointF startPos,
                QPointF startCtrlPos,
                QPointF endCtrlPos,
                QPointF endPos,
                qreal radius,
                QColor color);

    void drawOnImage(QImage *img) const;

    void prepareToDrawOnImage(QImage *img);
private:
    QPixmap mTexPix;
    qreal mStrokeTexHeight;
    qreal mStrokeTexWidth;
    int mMaxTexDabs;
    int mMaxStrokeDabs;
    int mNDabs;
    QPainter::PixmapFragment *mFragments;

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

class BrushEffect : public PixmapEffect
{
public:
    BrushEffect(qreal brushMinRadius = 0.,
                qreal brushMaxRadius = 0.,
                qreal strokeMinLength = 0.,
                qreal strokeMaxLength = 0.,
                qreal strokeMinDirectionAngle = 0.,
                qreal strokeMaxDirectionAngle = 0.,
                qreal strokeCurvature = 0.);

    void apply(QImage *imgPtr, const fmt_filters::image &img, qreal scale = 1.);

    qreal getMargin();
private:
    QrealAnimator mMinBrushRadius;
    QrealAnimator mMaxBrushRadius;
    QrealAnimator mStrokeMinLength;
    QrealAnimator mStrokeMaxLength;
    QrealAnimator mStrokeMinDirectionAngle;
    QrealAnimator mStrokeMaxDirectionAngle;
    QrealAnimator mStrokeCurvature;
};

#endif // PIXMAPEFFECT_H
