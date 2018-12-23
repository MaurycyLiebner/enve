#ifndef SKQTCONVERSIONS_H
#define SKQTCONVERSIONS_H
#include <QRectF>
#include <QMatrix>
#include <QColor>

#include "skiaincludes.h"

extern inline qreal SkScalarToQreal(const SkScalar &ss) {
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

extern inline QPointF SkPointToQPointF(const SkPoint &point) {
    return QPointF(SkScalarToQreal(point.x()),
                   SkScalarToQreal(point.y()));
}

extern inline SkPoint QPointFToSkPoint(const QPointF &point) {
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



//struct SkLine {
//    SkLine(const SkPoint &p1T,
//           const SkPoint &p2T) {
//        p1 = p1T;
//        p2 = p2T;
//    }

//    void setP1(const SkPoint &p1T) {
//        p1 = p1T;
//    }

//    void setP2(const SkPoint &p2T) {
//        p2 = p2T;
//    }

//    SkScalar dx() {
//        return p2.x() - p1.x();
//    }

//    SkScalar dy() {
//        return p2.y() - p1.y();
//    }

//    SkScalar length() const {
//        SkScalar x = p2.x() - p1.x();
//        SkScalar y = p2.y() - p1.y();
//        return SkScalarSqrt(x*x + y*y);
//    }

//    SkScalar angleTo(const SkLine &l) const;
//    SkScalar angle(const SkLine &l) const;

//    bool isNull() const;

//    SkPoint p1;
//    SkPoint p2;
//};

#endif // SKQTCONVERSIONS_H
