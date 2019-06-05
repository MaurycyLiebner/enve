#ifndef SKQTCONVERSIONS_H
#define SKQTCONVERSIONS_H

#include <QRectF>
#include <QMatrix>
#include <QColor>
#include "skiaincludes.h"

extern inline qreal toQreal(const SkScalar ss) {
    return static_cast<qreal>(ss);
}

extern inline SkScalar toSkScalar(const qreal qr) {
    return static_cast<SkScalar>(qr);
}

extern SkFont toSkFont(const QFont& qfont);
extern SkFont toSkFont(const QFont& qfont,
                       const int qPPI, const int skPPI);

extern SkColor toSkColor(const QColor& qcol);

extern QRectF toQRectF(const SkRect &rect);
extern SkRect toSkRect(const QRectF &rect);
// m11 - scaleX
// m12 - skewY
// m21 - skewX
// m22 - scaleY
extern QMatrix toQMatrix(const SkMatrix &matrix);

extern SkMatrix toSkMatrix(const QMatrix &matrix);

extern inline QPointF toQPointF(const SkPoint &point) {
    return QPointF(toQreal(point.x()),
                   toQreal(point.y()));
}

extern inline SkPoint toSkPoint(const QPointF &point) {
    return SkPoint::Make(toSkScalar(point.x()),
                         toSkScalar(point.y()));
}

extern inline SkPoint toSkPoint(const QPoint &point) {
    return SkPoint::Make(point.x(), point.y());
}

extern SkPaint::Cap QCapToSkCap(const Qt::PenCapStyle &cap);

extern SkPaint::Join QJoinToSkJoin(const Qt::PenJoinStyle &join);

extern SkPath toSkPath(const QPainterPath &qPath);
extern QPainterPath toQPainterPath(const SkPath &path);

extern void switchSkQ(const QPointF& qPos, SkPoint& skPos);
extern void switchSkQ(const SkPoint& skPos, QPointF& qPos);
extern void switchSkQ(const qreal q, SkScalar& sk);
extern void switchSkQ(const SkScalar sk, qreal& q);

#endif // SKQTCONVERSIONS_H
