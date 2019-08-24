// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "skqtconversions.h"

QRect toQRect(const SkIRect &rect) {
    return QRect(rect.left(), rect.top(),
                 rect.width(), rect.height());
}

SkIRect toSkIRect(const QRect &rect) {
    return SkIRect::MakeLTRB(rect.left(), rect.top(),
                             rect.right(), rect.bottom());
}

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
    SkPoint movePt{0, 0};
    SkPoint endPt{0, 0};
    SkPoint startPt{0, 0};
    const int iMax = qPath.elementCount() - 1;
    for(int i = 0; i <= iMax; i++) {
        const QPainterPath::Element &elem = qPath.elementAt(i);
        const SkPoint toPt{toSkScalar(elem.x), toSkScalar(elem.y)};
        if(elem.isMoveTo()) { // move
            movePt = toPt;
            path.moveTo(movePt);
        } else if(elem.isLineTo()) { // line
            path.lineTo(toPt);
        } else if(elem.isCurveTo()) { // curve
            endPt = toPt;
            firstOther = true;
        } else { // other
            if(firstOther) startPt = toPt;
            else path.cubicTo(endPt, startPt, toPt);
            firstOther = !firstOther;
        }
        if(i == iMax && SkPoint::Distance(movePt, toPt) < 0.001f)
            path.close();
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

void switchSkQ(const qreal q, float &sk) {
    sk = toSkScalar(q);
}

void switchSkQ(const float sk, qreal &q) {
    q = toQreal(sk);
}
#include <QFont>
SkFont toSkFont(const QFont &qfont, const int qPPI, const int skPPI) {
    SkFontStyle::Slant slant{SkFontStyle::kUpright_Slant};
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
    const SkFontStyle style(qfont.weight(),
                            SkFontStyle::kNormal_Width, slant);

    SkFont skFont;
    const auto fmlStdStr = qfont.family().toStdString();
    const auto typeface = SkTypeface::MakeFromName(fmlStdStr.c_str(), style);
    skFont.setTypeface(typeface);
    const qreal ptSize = qfont.pointSizeF()*qPPI/skPPI;
    skFont.setSize(toSkScalar(ptSize));
    return skFont;
}

SkFont toSkFont(const QFont& qfont) {
    return toSkFont(qfont, 72, 72);
}
