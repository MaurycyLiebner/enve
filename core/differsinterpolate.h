#ifndef DIFFERSINTERPOLATE_H
#define DIFFERSINTERPOLATE_H
#include <QString>
extern bool gDiffers(const QString& val1, const QString& val2);

extern bool gDiffers(const bool& val1, const bool& val2);

extern void gInterpolate(const qreal& val1, const qreal& val2,
                         const qreal& t, qreal& val);

class qCubicSegment1D;
extern bool gDiffers(const qCubicSegment1D& val1,
                     const qCubicSegment1D& val2);
extern void gInterpolate(const qCubicSegment1D& val1,
                         const qCubicSegment1D& val2,
                         const qreal& t,
                         qCubicSegment1D& val);

class SmartPath;
extern bool gDiffers(const SmartPath& path1, const SmartPath& path2);
extern void gInterpolate(const SmartPath& path1,
                         const SmartPath& path2,
                         const qreal& path2Weight,
                         SmartPath& target);

class BrushPolyline;
extern bool gDiffers(const BrushPolyline& path1, const BrushPolyline& path2);
extern void gInterpolate(const BrushPolyline& path1,
                         const BrushPolyline& path2,
                         const qreal& path2Weight,
                         BrushPolyline& target);

#endif // DIFFERSINTERPOLATE_H
