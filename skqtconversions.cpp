#include "skqtconversions.h"

QRectF SkRectToQRectF(const SkRect &rect) {
    return QRectF(rect.x(), rect.y(),
                  rect.width(), rect.height());
}

SkRect QRectFToSkRect(const QRectF &rect) {
    return SkRect::MakeXYWH(rect.x(), rect.y(),
                            rect.width(), rect.height());
}
// m11 - scaleX
// m12 - skewY
// m21 - skewX
// m22 - scaleY
QMatrix SkMatrixToQMatrix(const SkMatrix &matrix) {
    return QMatrix(matrix.getScaleX(), matrix.getSkewY(),
                   matrix.getSkewX(), matrix.getScaleY(),
                   matrix.getTranslateX(), matrix.getTranslateY());
}

SkMatrix QMatrixToSkMatrix(const QMatrix &matrix) {
    SkMatrix skMatrix;
    skMatrix.reset();
    skMatrix.set(SkMatrix::kMScaleX, matrix.m11());
    skMatrix.set(SkMatrix::kMScaleY, matrix.m22());
    skMatrix.set(SkMatrix::kMSkewX, matrix.m21());
    skMatrix.set(SkMatrix::kMSkewY, matrix.m12());
    skMatrix.set(SkMatrix::kMTransX, matrix.dx());
    skMatrix.set(SkMatrix::kMTransY, matrix.dy());
    return skMatrix;
}

QPointF SkPointToQPointF(const SkPoint &point) {
    return QPointF(point.x(), point.y());
}

SkPoint QPointFToSkPoint(const QPointF &point) {
    return SkPoint::Make(point.x(), point.y());
}

SkPaint::Cap QCapToSkCap(const Qt::PenCapStyle &cap) {
    if(cap == Qt::RoundCap) {
        return SkPaint::kRound_Cap;
    }
    if(cap == Qt::SquareCap) {
        return SkPaint::kSquare_Cap;
    }
    return SkPaint::kButt_Cap;
}

SkPaint::Join QJoinToSkJoin(const Qt::PenJoinStyle &join) {
    if(join == Qt::RoundJoin) {
        return SkPaint::kRound_Join;
    }
    if(join == Qt::BevelJoin) {
        return SkPaint::kBevel_Join;
    }
    return SkPaint::kMiter_Join;
}
