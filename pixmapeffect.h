#ifndef PIXMAPEFFECT_H
#define PIXMAPEFFECT_H
#include "fmt_filters.h"
#include "complexanimator.h"

class PixmapEffect : public ComplexAnimator
{
public:
    PixmapEffect();
    virtual void apply(QImage *imgPtr,
                       const fmt_filters::image &img, qreal scale,
                       bool highQuality) {}

    virtual qreal getMargin() {}
};

class BlurEffect : public PixmapEffect
{
public:
    BlurEffect(qreal radius = 10.);

    void apply(QImage *imgPtr, const fmt_filters::image &img, qreal scale,
               bool highQuality);

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

    static void loadStrokePixmaps();
private:
    static QList<QPixmap*> mStrokeTexPixmaps;
    static QPixmap *mEndPix;

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
    BrushEffect(qreal numberStrokes = 100.,
                qreal brushMinRadius = 20.,
                qreal brushMaxRadius = 30.,
                qreal strokeMaxLength = 200.,
                qreal strokeMinDirectionAngle = 0.,
                qreal strokeMaxDirectionAngle = 360.,
                qreal strokeCurvature = 0.5);

    void apply(QImage *imgPtr, const fmt_filters::image &img, qreal scale, bool highQuality);

    qreal getMargin();
private:
    QrealAnimator mMinBrushRadius;
    QrealAnimator mMaxBrushRadius;
    QrealAnimator mStrokeMaxLength;
    QrealAnimator mStrokeMinDirectionAngle;
    QrealAnimator mStrokeMaxDirectionAngle;
    QrealAnimator mStrokeCurvature;
    QrealAnimator mNumberStrokes;
};

class LinesEffect : public PixmapEffect
{
public:
    LinesEffect(qreal linesWidth = 5., qreal linesDistance = 5.);

    void apply(QImage *imgPtr, const fmt_filters::image &img, qreal scale, bool highQuality);

    qreal getMargin() { return 0.; }
private:
    QrealAnimator mLinesDistance;
    QrealAnimator mLinesWidth;
    bool mVertical = false;
};

class CirclesEffect : public PixmapEffect
{
public:
    CirclesEffect(qreal circlesRadius = 5., qreal circlesDistance = 5.);

    void apply(QImage *imgPtr, const fmt_filters::image &img, qreal scale, bool highQuality);

    qreal getMargin() { return 0.; }
private:
    QrealAnimator mCirclesDistance;
    QrealAnimator mCirclesRadius;
};

#endif // PIXMAPEFFECT_H
