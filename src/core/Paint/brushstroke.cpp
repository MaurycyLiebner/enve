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

#include "brushstroke.h"

QRect BrushStroke::execute(MyPaintBrush * const brush,
                           MyPaintSurface * const surface,
                           const bool press, double dLen) const {
    QRect changedRect;
    if(fUseColor) {
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_COLOR_H,
                                     fColor.hueF());
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_COLOR_S,
                                     fColor.saturationF());
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_COLOR_V,
                                     fColor.valueF());
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_OPAQUE,
                                     fColor.alphaF());
    }

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

QRect BrushStroke::executeMove(MyPaintBrush * const brush,
                               MyPaintSurface * const surface,
                               const double t, const double lenFrag) const {
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

QRect BrushStroke::executePress(MyPaintBrush * const brush,
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
                            xTilt, yTilt, 1);
    MyPaintRectangle roi;
    mypaint_surface_end_atomic(surface, &roi);
    return QRect(roi.x, roi.y, roi.width, roi.height);
}
