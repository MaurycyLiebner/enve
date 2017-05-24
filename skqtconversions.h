#ifndef SKQTCONVERSIONS_H
#define SKQTCONVERSIONS_H
#include <QRectF>
#include <QMatrix>
#include "SkMatrix.h"
#include "SkRect.h"

extern QRectF SkRectToQRectF(const SkRect &rect);
extern SkRect QRectFToSkRect(const QRectF &rect);
// m11 - scaleX
// m12 - skewY
// m21 - skewX
// m22 - scaleY
extern QMatrix SkMatrixToQMatrix(const SkMatrix &matrix);

extern SkMatrix QMatrixToSkMatrix(const QMatrix &matrix);

extern QPointF SkPointToQPointF(const SkPoint &point);

extern SkPoint QPointFToSkPoint(const QPointF &point);

#endif // SKQTCONVERSIONS_H
