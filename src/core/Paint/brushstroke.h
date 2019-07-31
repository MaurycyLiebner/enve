#ifndef BRUSHSTROKE_H
#define BRUSHSTROKE_H
#include "Segments/qcubicsegment1d.h"
#include "Segments/qcubicsegment2d.h"
#include "pointhelpers.h"
#include "pathoperations.h"
#include <QRect>
#include <mypaint-brush.h>
#define DefaultMoveStrokePressure(press) \
    BrushPressureCurve{press, press, press, press}
#define DefaultPressStrokePressure(iniPress, press, hardness) \
    BrushPressureCurve{iniPress, iniPress*(1 - hardness) + press*hardness, press, press}
#define DefaultTiltCurve \
    qCubicSegment1D{0, 0, 0, 0}
#define DefaultTimeCurve \
    qCubicSegment1D{1, 1, 1, 1}
struct BrushStroke {
    friend struct BrushStrokeSet;
    qCubicSegment2D fStrokePath;
    qCubicSegment1D fPressure;
    qCubicSegment1D fXTilt;
    qCubicSegment1D fYTilt;
    qCubicSegment1D fTimeCurve;
    qCubicSegment1D fWidthCurve;
    qCubicSegment1D fSpacingCurve;
private:
    QRect execute(MyPaintBrush * const brush,
                  MyPaintSurface * const surface,
                  const bool press,
                  double dLen) {
        QRect changedRect;
        if(press) changedRect = executePress(brush, surface);
        const double totalLength = fStrokePath.length();
        const int iMax = qCeil(totalLength/dLen);
        dLen = totalLength/iMax;
        const double lenFrag = 1./iMax;

        for(int i = 1; i <= iMax; i++) {
            const double t = fStrokePath.tAtLength(i*dLen);
            const QRect roi = executeMove(brush, surface, t, lenFrag);
            changedRect = changedRect.united(roi);
        }
        return changedRect;
    }

    QRect executeMove(MyPaintBrush * const brush,
                      MyPaintSurface * const surface,
                      const double t,
                      const double lenFrag) const {
        const QPointF pos = fStrokePath.posAtT(t);
        const qreal pressure = fPressure.valAtT(t);
        const qreal xTilt = fXTilt.valAtT(t);
        const qreal yTilt = fYTilt.valAtT(t);
        const qreal time = fTimeCurve.valAtT(t);
        const qreal width = fWidthCurve.valAtT(t);
        const qreal spacing = fSpacingCurve.valAtT(t);

        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_DABS_PER_ACTUAL_RADIUS,
                                     spacing);
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC,
                                     qLn(width));
        mypaint_surface_begin_atomic(surface);
        mypaint_brush_stroke_to(brush, surface, pos.x(), pos.y(), pressure,
                                xTilt, yTilt, time*lenFrag);
        MyPaintRectangle roi;
        mypaint_surface_end_atomic(surface, &roi);
        return QRect(roi.x, roi.y, roi.width, roi.height);
    }

    QRect executePress(MyPaintBrush * const brush,
                       MyPaintSurface * const surface) const {
        const QPointF pos = fStrokePath.p0();
        const qreal pressure = fPressure.p0();
        const qreal xTilt = fXTilt.p0();
        const qreal yTilt = fYTilt.p0();
        //qreal time = gCubicValueAtT(fTimeCurve, t);
        const qreal width = fWidthCurve.p0();
        const qreal spacing = fSpacingCurve.p0();

        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_DABS_PER_ACTUAL_RADIUS,
                                     spacing);
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC,
                                     qLn(width));

        mypaint_brush_reset(brush);
        mypaint_brush_new_stroke(brush);

        mypaint_surface_begin_atomic(surface);
        mypaint_brush_stroke_to(brush, surface, pos.x(), pos.y(), pressure,
                                xTilt, yTilt, 0.1);
        MyPaintRectangle roi;
        mypaint_surface_end_atomic(surface, &roi);
        return QRect(roi.x, roi.y, roi.width, roi.height);
    }
};

struct BrushStrokeSet {
    static BrushStrokeSet fromCubicList(CubicList& segs,
                                        qCubicSegment1D& timeCurve,
                                        qCubicSegment1D& pressureCurve,
                                        qCubicSegment1D& widthCurve,
                                        qCubicSegment1D& spacingCurve) {
        BrushStrokeSet set;
        qreal currLen = 0;
        qreal lastT = 0;
        for(auto& seg : segs) {
            currLen += seg.length();
            const qreal t = currLen/segs.getTotalLength();
            set.fStrokes << BrushStroke{seg,
                             pressureCurve.tFragment(lastT, t),
                             DefaultTiltCurve,
                             DefaultTiltCurve,
                             timeCurve.tFragment(lastT, t),
                             widthCurve.tFragment(lastT, t),
                             spacingCurve.tFragment(lastT, t)};
            lastT = t;
        }
        return set;
    }

    static QList<BrushStrokeSet> fromSkPath(const SkPath& path,
                                            qCubicSegment1D& timeCurve,
                                            qCubicSegment1D& pressureCurve,
                                            qCubicSegment1D& widthCurve,
                                            qCubicSegment1D& spacingCurve) {
        QList<BrushStrokeSet> result;

        auto segLists = CubicList::sMakeFromSkPath(path);
        if(segLists.isEmpty()) return result;
        for(auto& segs : segLists) {
            if(segs.isEmpty()) continue;
            const double minL = 0;
            const double maxL = segs.isClosed() ? 1 + 10/segs.getTotalLength() : 1;
            auto segsT = segs.getFragmentUnbound(minL, maxL);
            result << fromCubicList(segsT, timeCurve,
                                    pressureCurve, widthCurve, spacingCurve);
        }
        return result;
    }

    static QList<BrushStrokeSet> lineFillStrokesForSkPath(
            const SkPath& path,
            qCubicSegment1D& timeCurve,
            qCubicSegment1D& pressureCurve,
            qCubicSegment1D& widthCurve,
            qCubicSegment1D& spacingCurve,
            const qreal degAngle,
            const qreal distInc) {
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
                                   DefaultTiltCurve, DefaultTiltCurve,
                                   timeCurve, widthCurve, spacingCurve};
                lineSet.fStrokes.append(stroke);
                result << lineSet;
            }
        }
        return result;
    }

    static QList<BrushStrokeSet> fillStrokesForSkPath(
            const SkPath& path,
            qCubicSegment1D& timeCurve,
            qCubicSegment1D& pressureCurve,
            qCubicSegment1D& widthCurve,
            qCubicSegment1D& spacingCurve,
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
                result << BrushStrokeSet::fromSkPath(strokePath, timeCurve,
                                                     pressureCurve, widthCurve,
                                                     spacingCurve);
            }
            //strokePath = gSmoothyPath(strokePath, 1/*static_cast<float>(i - 1)/maxI*/);

        }
        return result;
    }

    static QList<BrushStrokeSet> outlineStrokesForSkPath(
            const SkPath& path,
            qCubicSegment1D& timeCurve,
            qCubicSegment1D& pressureCurve,
            qCubicSegment1D& widthCurve,
            qCubicSegment1D& spacingCurve,
            const qreal distInc,
            const qreal outlineWidth) {
        QList<BrushStrokeSet> result;
        result << BrushStrokeSet::fromSkPath(path, timeCurve, pressureCurve,
                                             widthCurve, spacingCurve);
        qreal halfWidth = 0.5*outlineWidth;
        qreal min = -halfWidth + distInc*0.5;
        qreal max = halfWidth - distInc*0.5;
        for(qreal dist = min; dist < max; dist += distInc*0.5) {
            SkPath strokePath;
            gSolidify(dist, path, &strokePath);
            if(strokePath.isEmpty()) continue;
            result << BrushStrokeSet::fromSkPath(strokePath,
                                                 timeCurve,
                                                 pressureCurve,
                                                 widthCurve,
                                                 spacingCurve);
        }
        return result;
    }

    QRect execute(MyPaintBrush * const brush,
                  MyPaintSurface * const surface,
                  const double dLen) {
        if(fStrokes.isEmpty()) return QRect();
        QRect updateRect = fStrokes[0].execute(brush, surface, true, dLen);
        for(int i = 1; i < fStrokes.count(); i++) {
            auto& stroke = fStrokes[i];
            const QRect roi = stroke.execute(brush, surface, false, dLen);
            updateRect = updateRect.united(roi);
        }
        return updateRect;
    }
    QList<BrushStroke> fStrokes;
    bool fClosed;
};

#endif // BRUSHSTROKE_H
