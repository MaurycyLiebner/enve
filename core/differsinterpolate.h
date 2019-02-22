#ifndef DIFFERSINTERPOLATE_H
#define DIFFERSINTERPOLATE_H
#include <QString>
class qCubicSegment1D;
class PathBase;
extern bool gDiffers(const QString& val1, const QString& val2);
extern bool gDiffers(const bool& val1, const bool& val2);

extern void gInterpolate(const qreal& val1, const qreal& val2,
                         const qreal& t, qreal& val);

extern bool gDiffers(const qCubicSegment1D& val1,
                     const qCubicSegment1D& val2);
extern void gInterpolate(const qCubicSegment1D& val1,
                         const qCubicSegment1D& val2,
                         const qreal& t,
                         qCubicSegment1D& val);


extern bool gDiffers(const PathBase& path1, const PathBase& path2);

#endif // DIFFERSINTERPOLATE_H
