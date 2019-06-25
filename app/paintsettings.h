#ifndef PAINTSETTINGS_H
#define PAINTSETTINGS_H

enum PaintType : short {
    NOPAINT,
    FLATPAINT,
    GRADIENTPAINT,
    BRUSHPAINT
};

enum ColorMode : short;
#include <QColor>
struct ColorSettings {
    QColor fColor;
    ColorMode fMode;
};

class Gradient;
enum class GradientType : short;
#include "smartPointers/sharedpointerdefs.h"
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

struct StrokeSettings : public FillSettings {
    qreal fWidth;
    SkPaint::Cap fCapStyle;
    SkPaint::Join fJoinStyle;
    BrushSettings fBrush;
};

#endif // PAINTSETTINGS_H
