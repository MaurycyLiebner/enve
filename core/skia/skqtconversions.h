#ifndef SKQTCONVERSIONS_H
#define SKQTCONVERSIONS_H

#include <QRectF>
#include <QMatrix>
#include <QColor>
#include "skiaincludes.h"

extern inline qreal skScalarToQ(const SkScalar &ss) {
    return static_cast<qreal>(ss);
}

extern inline SkScalar qrealToSkScalar(const qreal &qr) {
    return static_cast<SkScalar>(qr);
}


extern SkColor QColorToSkColor(const QColor& qcol);

extern QRectF SkRectToQRectF(const SkRect &rect);
extern SkRect QRectFToSkRect(const QRectF &rect);
// m11 - scaleX
// m12 - skewY
// m21 - skewX
// m22 - scaleY
extern QMatrix SkMatrixToQMatrix(const SkMatrix &matrix);

extern SkMatrix QMatrixToSkMatrix(const QMatrix &matrix);

extern inline QPointF skPointToQ(const SkPoint &point) {
    return QPointF(skScalarToQ(point.x()),
                   skScalarToQ(point.y()));
}

extern inline SkPoint qPointToSk(const QPointF &point) {
    return SkPoint::Make(qrealToSkScalar(point.x()),
                         qrealToSkScalar(point.y()));
}

extern inline SkPoint QPointToSkPoint(const QPoint &point) {
    return SkPoint::Make(point.x(), point.y());
}

extern SkPaint::Cap QCapToSkCap(const Qt::PenCapStyle &cap);

extern SkPaint::Join QJoinToSkJoin(const Qt::PenJoinStyle &join);

extern bool skRectContainesSkPoint(const SkRect &absRect,
                                   const SkPoint &pos);
extern SkPath QPainterPathToSkPath(const QPainterPath &qPath);
extern QPainterPath SkPathToQPainterPath(const SkPath &path);

extern void switchSkQ(const QPointF& qPos, SkPoint& skPos);
extern void switchSkQ(const SkPoint& skPos, QPointF& qPos);
extern void switchSkQ(const qreal& q, SkScalar& sk);
extern void switchSkQ(const SkScalar& sk, qreal& q);

#endif // SKQTCONVERSIONS_H
