#ifndef BRUSHEFFECT_H
#define BRUSHEFFECT_H
#include "pixmapeffect.h"

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
    static bool mBrushPixmapsLoaded;
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
               qreal scale,
               bool highQuality);

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

#endif // BRUSHEFFECT_H
