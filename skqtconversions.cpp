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

SkPoint QPointToSkPoint(const QPoint &point) {
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
#include <QPainterPath>
SkPath QPainterPathToSkPath(const QPainterPath &qPath) {
    SkPath path;
    bool firstOther;
    SkPoint endPt;
    SkPoint startPt;
    for(int i = 0; i < qPath.elementCount(); i++) {
        const QPainterPath::Element &elem = qPath.elementAt(i);

        if(elem.isMoveTo()) { // move
            path.moveTo(elem.x, elem.y);
        } else if(elem.isLineTo()) { // line
            path.lineTo(elem.x, elem.y);
        } else if(elem.isCurveTo()) { // curve
            endPt = SkPoint::Make(elem.x, elem.y);
            firstOther = true;
        } else { // other
            if(firstOther) {
                startPt = SkPoint::Make(elem.x, elem.y);
            } else {
                path.cubicTo(endPt, startPt, SkPoint::Make(elem.x, elem.y));
            }
            firstOther = !firstOther;
        }
    }
    return path;
}

QPainterPath SkPathToQPainterPath(const SkPath& path) {
    QPainterPath qPath;
    SkPath::RawIter iter = SkPath::RawIter(path);

    SkPoint pts[4];
    for(;;) {
        switch(iter.next(pts)) {
            case SkPath::kMove_Verb: {
                SkPoint pt = pts[0];
                qPath.moveTo(SkPointToQPointF(pt));
            }
                break;
            case SkPath::kLine_Verb: {
                SkPoint pt = pts[1];

                qPath.lineTo(SkPointToQPointF(pt));
            }
                break;
            case SkPath::kCubic_Verb: {
                SkPoint endPt = pts[1];
                SkPoint startPt = pts[2];
                SkPoint targetPt = pts[3];
                qPath.cubicTo(SkPointToQPointF(endPt),
                              SkPointToQPointF(startPt),
                              SkPointToQPointF(targetPt));
            }
                break;
            case SkPath::kClose_Verb:
                qPath.closeSubpath();
                break;
            case SkPath::kQuad_Verb:
            case SkPath::kConic_Verb:
            case SkPath::kDone_Verb:
                return qPath;
                break;
        }
    }
    return qPath;
}

//SkScalar SkLine::angleTo(const SkLine &l) const {
//    if(isNull() || l.isNull()) return 0.f;
//    const SkScalar a1 = angle();
//    const SkScalar a2 = l.angle();

//    const SkScalar delta = a2 - a1;
//    const SkScalar delta_normalized = delta < 0 ? delta + 360 : delta;

//    if(qFuzzyCompare(delta, SkScalar(360))) {
//        return 0;
//    } else {
//        return delta_normalized;
//    }
//}

//bool SkLine::isNull() const {
//    return (qFuzzyCompare(pt1.x(), pt2.x()) &&
//            qFuzzyCompare(pt1.y(), pt2.y())) ? true : false;
//}

//SkScalar SkLine::angle(const SkLine &l) const {
//    if(isNull() || l.isNull) return 0.f;
//    SkScalar cos_line = (dx()*l.dx() + dy()*l.dy()) / (length()*l.length());
//    SkScalar rad = 0;

//    if(cos_line >= SkScalar(-1.f) &&
//       cos_line <= SkScalar(1.f)) {
//        rad = SkScalarACos(cos_line);
//    }
//    return SkRadiansToDegrees(rad);
//}

//bool skRectContainesSkPoint(const SkRect &absRect,
//                            const SkPoint &pos) {
//    if(absRect.bottom() < pos.y()) return false;
//    if(absRect.top() > pos.y()) return false;
//    if(absRect.right() < pos.x()) return false;
//    if(absRect.left() > pos.x()) return false;
//    return true;
//}
