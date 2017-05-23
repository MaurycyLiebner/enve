#ifndef SKQTCONVERSIONS_H
#define SKQTCONVERSIONS_H
#include <QRectF>
#include <QMatrix>
#include "SkMatrix.h"
#include "SkRect.h"

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
    skMatrix.setScale(matrix.m11(), matrix.m22());
    skMatrix.setSkew(matrix.m21(), matrix.m12());
    skMatrix.setTranslate(matrix.dx(), matrix.dy());
    return skMatrix;
}

QPointF SkPointToQPointF(const SkPoint &point) {
    return QPointF(point.x(), point.y());
}

SkPoint QPointFToSkPoint(const QPointF &point) {
    return SkPoint::Make(point.x(), point.y());
}

#endif // SKQTCONVERSIONS_H
