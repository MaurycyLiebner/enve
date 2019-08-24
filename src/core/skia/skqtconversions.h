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

#ifndef SKQTCONVERSIONS_H
#define SKQTCONVERSIONS_H

#include <QRectF>
#include <QMatrix>
#include <QColor>
#include "skiaincludes.h"

extern inline qreal toQreal(const float ss) {
    return static_cast<qreal>(ss);
}

extern inline float toSkScalar(const qreal qr) {
    return static_cast<float>(qr);
}

extern SkFont toSkFont(const QFont& qfont);
extern SkFont toSkFont(const QFont& qfont,
                       const int qPPI, const int skPPI);

extern SkColor toSkColor(const QColor& qcol);

extern QRect toQRect(const SkIRect &rect);
extern SkIRect toSkIRect(const QRect &rect);

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
extern void switchSkQ(const qreal q, float& sk);
extern void switchSkQ(const float sk, qreal& q);

#endif // SKQTCONVERSIONS_H
