// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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

#ifndef BRUSHSTROKESET_H
#define BRUSHSTROKESET_H

#include "brushstroke.h"

struct CORE_EXPORT BrushStrokeSet {
    static BrushStrokeSet sFromCubicList(
            const CubicList& segs,
            const qCubicSegment1D& timeCurve,
            const qCubicSegment1D& pressureCurve,
            const qCubicSegment1D& widthCurve,
            const qCubicSegment1D& spacingCurve);

    static QList<BrushStrokeSet> sFromSkPath(
            const SkPath& path,
            const qCubicSegment1D &timeCurve,
            const qCubicSegment1D &pressureCurve,
            const qCubicSegment1D &widthCurve,
            const qCubicSegment1D &spacingCurve);

    static QList<BrushStrokeSet> sLineFillStrokesForSkPath(
            const SkPath& path,
            const qCubicSegment1D& timeCurve,
            const qCubicSegment1D& pressureCurve,
            const qCubicSegment1D& widthCurve,
            const qCubicSegment1D& spacingCurve,
            const qreal degAngle,
            const qreal distInc);

    static QList<BrushStrokeSet> sFillStrokesForSkPath(const SkPath& path,
            const qCubicSegment1D &timeCurve,
            const qCubicSegment1D &pressureCurve,
            const qCubicSegment1D &widthCurve,
            const qCubicSegment1D &spacingCurve,
            const qreal distInc);

    static QList<BrushStrokeSet> sOutlineStrokesForSkPath(
            const SkPath& path,
            const qCubicSegment1D& timeCurve,
            const qCubicSegment1D& pressureCurve,
            const qCubicSegment1D& widthCurve,
            const qCubicSegment1D& spacingCurve,
            const qreal distInc,
            const qreal outlineWidth);

    QRect execute(MyPaintBrush * const brush,
                  MyPaintSurface * const surface,
                  const double dLen) const;

    QList<BrushStroke> fStrokes;
    bool fClosed;
};

#endif // BRUSHSTROKESET_H
