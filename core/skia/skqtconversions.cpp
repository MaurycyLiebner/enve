#include "skqtconversions.h"

QRectF toQRectF(const SkRect &rect) {
    return QRectF(toQreal(rect.x()),
                  toQreal(rect.y()),
                  toQreal(rect.width()),
                  toQreal(rect.height()));
}

SkRect toSkRect(const QRectF &rect) {
    return SkRect::MakeXYWH(toSkScalar(rect.x()),
                            toSkScalar(rect.y()),
                            toSkScalar(rect.width()),
                            toSkScalar(rect.height()));
}
// m11 - scaleX
// m12 - skewY
// m21 - skewX
// m22 - scaleY
QMatrix toQMatrix(const SkMatrix &matrix) {
    return QMatrix(toQreal(matrix.getScaleX()),
                   toQreal(matrix.getSkewY()),
                   toQreal(matrix.getSkewX()),
                   toQreal(matrix.getScaleY()),
                   toQreal(matrix.getTranslateX()),
                   toQreal(matrix.getTranslateY()));
}

SkMatrix toSkMatrix(const QMatrix &matrix) {
    SkMatrix skMatrix;
    skMatrix.reset();
    skMatrix.set(SkMatrix::kMScaleX, toSkScalar(matrix.m11()));
    skMatrix.set(SkMatrix::kMScaleY, toSkScalar(matrix.m22()));
    skMatrix.set(SkMatrix::kMSkewX, toSkScalar(matrix.m21()));
    skMatrix.set(SkMatrix::kMSkewY, toSkScalar(matrix.m12()));
    skMatrix.set(SkMatrix::kMTransX, toSkScalar(matrix.dx()));
    skMatrix.set(SkMatrix::kMTransY, toSkScalar(matrix.dy()));
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
SkPath toSkPath(const QPainterPath &qPath) {
    SkPath path;
    bool firstOther = false;
    SkPoint endPt;
    SkPoint startPt;
    for(int i = 0; i < qPath.elementCount(); i++) {
        const QPainterPath::Element &elem = qPath.elementAt(i);

        if(elem.isMoveTo()) { // move
            path.moveTo(toSkScalar(elem.x),
                        toSkScalar(elem.y));
        } else if(elem.isLineTo()) { // line
            path.lineTo(toSkScalar(elem.x),
                        toSkScalar(elem.y));
        } else if(elem.isCurveTo()) { // curve
            endPt = SkPoint::Make(toSkScalar(elem.x),
                                  toSkScalar(elem.y));
            firstOther = true;
        } else { // other
            if(firstOther) {
                startPt = SkPoint::Make(toSkScalar(elem.x),
                                        toSkScalar(elem.y));
            } else {
                path.cubicTo(endPt, startPt,
                             SkPoint::Make(toSkScalar(elem.x),
                                           toSkScalar(elem.y)));
            }
            firstOther = !firstOther;
        }
    }
    return path;
}

QPainterPath toQPainterPath(const SkPath& path) {
    QPainterPath qPath;
    SkPath::RawIter iter = SkPath::RawIter(path);

    SkPoint pts[4];
    for(;;) {
        auto nextVerb = iter.next(pts);
        switch(nextVerb) {
            case SkPath::kMove_Verb: {
                const SkPoint pt = pts[0];
                qPath.moveTo(toQPointF(pt));
            }
                break;
            case SkPath::kLine_Verb: {
                const SkPoint pt = pts[1];

                qPath.lineTo(toQPointF(pt));
            }
                break;
            case SkPath::kCubic_Verb: {
                const SkPoint endPt = pts[1];
                const SkPoint startPt = pts[2];
                const SkPoint targetPt = pts[3];
                qPath.cubicTo(toQPointF(endPt),
                              toQPointF(startPt),
                              toQPointF(targetPt));
            }
                break;
            case SkPath::kClose_Verb:
                qPath.closeSubpath();
                break;
            case SkPath::kQuad_Verb: {
                const SkPoint ctrlPt = pts[1];
                const SkPoint targetPt = pts[2];
                qPath.quadTo(toQPointF(ctrlPt),
                             toQPointF(targetPt));
            }
                break;
            case SkPath::kConic_Verb: {
                const QPointF p0 = qPath.currentPosition();
                const QPointF p1 = toQPointF(pts[1]);
                const QPointF p2 = toQPointF(pts[2]);
                const qreal weight = SkScalarToDouble(iter.conicWeight());

                const qreal u = 4*weight/(3*(1 + weight));
                qPath.cubicTo(p0*(1 - u) + p1*u, p2*(1 - u) + p1*u, p2);
            }
            break;
            case SkPath::kDone_Verb:
                return qPath;
        }
    }
}

SkColor toSkColor(const QColor &qcol) {
    return SkColorSetARGB(static_cast<U8CPU>(qcol.alpha()),
                          static_cast<U8CPU>(qcol.red()),
                          static_cast<U8CPU>(qcol.green()),
                          static_cast<U8CPU>(qcol.blue()));
}

void switchSkQ(const QPointF &qPos, SkPoint &skPos) {
    skPos = toSkPoint(qPos);
}

void switchSkQ(const SkPoint &skPos, QPointF &qPos) {
    qPos = toQPointF(skPos);
}

void switchSkQ(const qreal &q, SkScalar &sk) {
    sk = toSkScalar(q);
}

void switchSkQ(const SkScalar &sk, qreal &q) {
    q = toQreal(sk);
}
#include <QFont>
SkFont toSkFont(const QFont &qfont) {
    SkFontStyle::Slant slant;
    switch(qfont.style()) {
    case QFont::StyleOblique:
        slant = SkFontStyle::kOblique_Slant;
        break;
    case QFont::StyleItalic:
        slant = SkFontStyle::kItalic_Slant;
        break;
    case QFont::StyleNormal:
        slant = SkFontStyle::kUpright_Slant;
        break;
    }
    SkFontStyle style(qfont.weight()*10, SkFontStyle::kNormal_Width, slant);

    SkFont skFont;
    const auto fmlStdStr = qfont.family().toStdString();
    const auto typeface = SkTypeface::MakeFromName(fmlStdStr.c_str(), style);
    skFont.setTypeface(typeface);
    skFont.setSize(toSkScalar(qfont.pointSizeF()));
    return skFont;
}
