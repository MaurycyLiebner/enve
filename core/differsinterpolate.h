#ifndef DIFFERSINTERPOLATE_H
#define DIFFERSINTERPOLATE_H
#include <QString>
#include "Segments/qcubicsegment1d.h"

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


#endif // DIFFERSINTERPOLATE_H
