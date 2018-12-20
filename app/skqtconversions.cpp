#include "skqtconversions.h"

QRectF SkRectToQRectF(const SkRect &rect) {
    return QRectF(SkScalarToQreal(rect.x()),
                  SkScalarToQreal(rect.y()),
                  SkScalarToQreal(rect.width()),
                  SkScalarToQreal(rect.height()));
}

SkRect QRectFToSkRect(const QRectF &rect) {
    return SkRect::MakeXYWH(qrealToSkScalar(rect.x()),
                            qrealToSkScalar(rect.y()),
                            qrealToSkScalar(rect.width()),
                            qrealToSkScalar(rect.height()));
}
// m11 - scaleX
// m12 - skewY
// m21 - skewX
// m22 - scaleY
QMatrix SkMatrixToQMatrix(const SkMatrix &matrix) {
    return QMatrix(SkScalarToQreal(matrix.getScaleX()),
                   SkScalarToQreal(matrix.getSkewY()),
                   SkScalarToQreal(matrix.getSkewX()),
                   SkScalarToQreal(matrix.getScaleY()),
                   SkScalarToQreal(matrix.getTranslateX()),
                   SkScalarToQreal(matrix.getTranslateY()));
}

SkMatrix QMatrixToSkMatrix(const QMatrix &matrix) {
    SkMatrix skMatrix;
    skMatrix.reset();
    skMatrix.set(SkMatrix::kMScaleX, qrealToSkScalar(matrix.m11()));
    skMatrix.set(SkMatrix::kMScaleY, qrealToSkScalar(matrix.m22()));
    skMatrix.set(SkMatrix::kMSkewX, qrealToSkScalar(matrix.m21()));
    skMatrix.set(SkMatrix::kMSkewY, qrealToSkScalar(matrix.m12()));
    skMatrix.set(SkMatrix::kMTransX, qrealToSkScalar(matrix.dx()));
    skMatrix.set(SkMatrix::kMTransY, qrealToSkScalar(matrix.dy()));
    return skMatrix;
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
    bool firstOther = false;
    SkPoint endPt;
    SkPoint startPt;
    for(int i = 0; i < qPath.elementCount(); i++) {
        const QPainterPath::Element &elem = qPath.elementAt(i);

        if(elem.isMoveTo()) { // move
            path.moveTo(qrealToSkScalar(elem.x),
                        qrealToSkScalar(elem.y));
        } else if(elem.isLineTo()) { // line
            path.lineTo(qrealToSkScalar(elem.x),
                        qrealToSkScalar(elem.y));
        } else if(elem.isCurveTo()) { // curve
            endPt = SkPoint::Make(qrealToSkScalar(elem.x),
                                  qrealToSkScalar(elem.y));
            firstOther = true;
        } else { // other
            if(firstOther) {
                startPt = SkPoint::Make(qrealToSkScalar(elem.x),
                                        qrealToSkScalar(elem.y));
            } else {
                path.cubicTo(endPt, startPt,
                             SkPoint::Make(qrealToSkScalar(elem.x),
                                           qrealToSkScalar(elem.y)));
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
        auto nextVerb = iter.next(pts);
        switch(nextVerb) {
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
            case SkPath::kQuad_Verb: {
                SkPoint ctrlPt = pts[1];
                SkPoint targetPt = pts[2];
                qPath.quadTo(SkPointToQPointF(ctrlPt),
                             SkPointToQPointF(targetPt));
            }
                break;
            case SkPath::kConic_Verb: {
                SkPoint ctrlPt = pts[1];
                SkPoint targetPt = pts[2];
                qPath.quadTo(SkPointToQPointF(ctrlPt),
                             SkPointToQPointF(targetPt));
            }
            break;
            case SkPath::kDone_Verb:
                return qPath;
        }
    }
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

SkColor QColorToSkColor(const QColor &qcol) {
    return SkColorSetARGB(static_cast<U8CPU>(qcol.alpha()),
                          static_cast<U8CPU>(qcol.red()),
                          static_cast<U8CPU>(qcol.green()),
                          static_cast<U8CPU>(qcol.blue()));
}
