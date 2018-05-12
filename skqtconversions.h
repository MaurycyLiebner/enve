#ifndef SKQTCONVERSIONS_H
#define SKQTCONVERSIONS_H
#include <QRectF>
#include <QMatrix>
#include "skiaincludes.h"

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

extern SkPoint QPointToSkPoint(const QPoint &point);

extern SkPaint::Cap QCapToSkCap(const Qt::PenCapStyle &cap);

extern SkPaint::Join QJoinToSkJoin(const Qt::PenJoinStyle &join);

extern bool skRectContainesSkPoint(const SkRect &absRect,
                                   const SkPoint &pos);
extern SkPath QPainterPathToSkPath(const QPainterPath &qPath);
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
