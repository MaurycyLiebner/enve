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
