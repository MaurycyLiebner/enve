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

#ifndef PAINTSETTINGS_H
#define PAINTSETTINGS_H

enum PaintType : short {
    NOPAINT,
    FLATPAINT,
    GRADIENTPAINT,
    BRUSHPAINT
};

enum class ColorMode : short;
#include <QColor>
struct ColorSettings {
    QColor fColor;
    ColorMode fMode;
};

class Gradient;
enum class GradientType : short;
#include "smartPointers/ememory.h"
struct GradientSettings {
    GradientType fType;
    qptr<Gradient> fGradient;
};

struct FillSettings {
    PaintType fType;
    ColorSettings fColor;
    GradientSettings fGradient;
};

class SimpleBrushWrapper;
#include "Segments/qcubicsegment1d.h"
struct BrushSettings {
    SimpleBrushWrapper* fBrush = nullptr;
    qCubicSegment1D fWidth;
    qCubicSegment1D fPressure;
    qCubicSegment1D fSpacing;
    qCubicSegment1D fTime;
};

#include "skia/skiaincludes.h"
struct StrokeSettings : public FillSettings {
    qreal fWidth;
    SkPaint::Cap fCapStyle;
    SkPaint::Join fJoinStyle;
    BrushSettings fBrush;
};

#endif // PAINTSETTINGS_H
