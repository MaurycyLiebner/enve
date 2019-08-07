#ifndef PAINTSETTINGS_H
#define PAINTSETTINGS_H

enum PaintType : short {
    NOPAINT,
    FLATPAINT,
    GRADIENTPAINT
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

#include "skia/skiaincludes.h"
struct StrokeSettings : public FillSettings {
    qreal fWidth;
    SkPaint::Cap fCapStyle;
    SkPaint::Join fJoinStyle;
};

#endif // PAINTSETTINGS_H
