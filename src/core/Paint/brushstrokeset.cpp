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

#include "brushstrokeset.h"

BrushStrokeSet BrushStrokeSet::sFromCubicList(
        const CubicList &segs,
        const qCubicSegment1D &timeCurve,
        const qCubicSegment1D &pressureCurve,
        const qCubicSegment1D &widthCurve,
        const qCubicSegment1D &spacingCurve) {
    BrushStrokeSet set;
    qreal currLen = 0;
    qreal lastT = 0;
    for(const auto& seg : segs) {
        currLen += seg.length();
        const qreal t = currLen/segs.getTotalLength();
        set.fStrokes << BrushStroke{seg,
                        pressureCurve.tFragment(lastT, t),
                        DEFAULT_TILT_CURVE,
                        DEFAULT_TILT_CURVE,
                        timeCurve.tFragment(lastT, t),
                        widthCurve.tFragment(lastT, t),
                        spacingCurve.tFragment(lastT, t)};
        lastT = t;
    }
    return set;
}

QList<BrushStrokeSet> BrushStrokeSet::sFromSkPath(
        const SkPath &path,
        const qCubicSegment1D &timeCurve,
        const qCubicSegment1D &pressureCurve,
        const qCubicSegment1D &widthCurve,
        const qCubicSegment1D &spacingCurve) {
    QList<BrushStrokeSet> result;

    auto segLists = CubicList::sMakeFromSkPath(path);
    if(segLists.isEmpty()) return result;
    for(auto& segs : segLists) {
        if(segs.isEmpty()) continue;
        const double totLen = segs.getTotalLength();
        if(isZero4Dec(totLen)) continue;
        const double minL = 0;
        const double maxL = segs.isClosed() ? 1 + 10/totLen : 1;
        auto segsT = segs.getFragmentUnbound(minL, maxL);
        result << sFromCubicList(segsT, timeCurve,
                                 pressureCurve, widthCurve, spacingCurve);
    }
    return result;
}

QList<BrushStrokeSet> BrushStrokeSet::sLineFillStrokesForSkPath(
        const SkPath &path,
        const qCubicSegment1D &timeCurve,
        const qCubicSegment1D &pressureCurve,
        const qCubicSegment1D &widthCurve,
        const qCubicSegment1D &spacingCurve,
        const qreal degAngle, const qreal distInc) {
    QList<BrushStrokeSet> result;
    auto segLists = CubicList::sMakeFromSkPath(path);
    const QRectF pathBounds = toQRectF(path.getBounds());
    QTransform rotate;
    rotate.rotate(degAngle);
    const QPolygonF linesBBPolygon = rotate.map(QPolygonF(pathBounds));
    const QLineF firstLine(linesBBPolygon.at(0), linesBBPolygon.at(1));
    const QLineF sideLine(linesBBPolygon.at(1), linesBBPolygon.at(2));
    const int nLines = qCeil(sideLine.length()/distInc);
    const QLineF transVec = QLineF::fromPolar(distInc, degAngle - 90);
    for(int i = 0; i < nLines; i++) {
        const QLineF iLine = firstLine.translated(i*transVec.p2());
        QList<QPointF> intersections;
        for(auto& seg : segLists) {
            //                intersections.append(iLine.p1());
            //                intersections.append(iLine.p2());
            //                continue;
            seg.lineIntersections(iLine, intersections);
        }

        const int jMin = 0;
        const int jMax = intersections.count() - 2;
        for(int j = jMin; j <= jMax; j += 2) {
            const QLineF line(intersections.at(j), intersections.at(j + 1));
            BrushStrokeSet lineSet;
            BrushStroke stroke{qCubicSegment2D::sFromLine(line), pressureCurve,
                        DEFAULT_TILT_CURVE, DEFAULT_TILT_CURVE,
                        timeCurve, widthCurve, spacingCurve};
            lineSet.fStrokes.append(stroke);
            result << lineSet;
        }
    }
    return result;
}

QList<BrushStrokeSet> BrushStrokeSet::sFillStrokesForSkPath(
        const SkPath &path,
        const qCubicSegment1D &timeCurve,
        const qCubicSegment1D &pressureCurve,
        const qCubicSegment1D &widthCurve,
        const qCubicSegment1D &spacingCurve,
        const qreal distInc) {
    auto pathBounds = path.getBounds();
    int maxI = qMax(qCeil(static_cast<double>(pathBounds.width())/distInc),
                    qCeil(static_cast<double>(pathBounds.height())/distInc));
    QList<BrushStrokeSet> result;
    //result << BrushStrokeSet::fromSkPath(path);
    for(int i = 1; i < maxI; i++) {
        SkPath strokePath;
        gSolidify(-i*distInc, path, &strokePath);
        //            gDisplaceFilterPath(&strokePath, SkPath(strokePath),
        //                                1, 100, 1, 1);
        for(int j = 0; j < 10; j++) {
            if(strokePath.isEmpty()) continue;
            result << BrushStrokeSet::sFromSkPath(strokePath, timeCurve,
                                                  pressureCurve, widthCurve,
                                                  spacingCurve);
        }
        //strokePath = gSmoothyPath(strokePath, 1/*static_cast<float>(i - 1)/maxI*/);

    }
    return result;
}

QList<BrushStrokeSet> BrushStrokeSet::sOutlineStrokesForSkPath(
        const SkPath &path,
        const qCubicSegment1D &timeCurve,
        const qCubicSegment1D &pressureCurve,
        const qCubicSegment1D &widthCurve,
        const qCubicSegment1D &spacingCurve,
        const qreal distInc, const qreal outlineWidth) {
    QList<BrushStrokeSet> result;
    result << BrushStrokeSet::sFromSkPath(path, timeCurve, pressureCurve,
                                          widthCurve, spacingCurve);
    qreal halfWidth = 0.5*outlineWidth;
    qreal min = -halfWidth + distInc*0.5;
    qreal max = halfWidth - distInc*0.5;
    for(qreal dist = min; dist < max; dist += distInc*0.5) {
        SkPath strokePath;
        gSolidify(dist, path, &strokePath);
        if(strokePath.isEmpty()) continue;
        result << BrushStrokeSet::sFromSkPath(strokePath,
                                              timeCurve,
                                              pressureCurve,
                                              widthCurve,
                                              spacingCurve);
    }
    return result;
}

QRect BrushStrokeSet::execute(MyPaintBrush * const brush,
                              MyPaintSurface * const surface,
                              const double dLen) const {
    if(fStrokes.isEmpty()) return QRect();
    QRect updateRect = fStrokes[0].execute(brush, surface, true, dLen);
    for(int i = 1; i < fStrokes.count(); i++) {
        auto& stroke = fStrokes[i];
        const QRect roi = stroke.execute(brush, surface, false, dLen);
        updateRect = updateRect.united(roi);
    }
    return updateRect;
}
