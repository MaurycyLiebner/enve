#ifndef PATHOPERATIONS_H
#define PATHOPERATIONS_H
#include <QPainterPath>
class PathBox;
class SkPath;
class PathAnimator;

extern void gApplyOperation(const qreal relFrame, const SkPath &src,
                            SkPath * const dst, PathBox * const srcBox,
                            PathBox * const dstBox, const QString &operation);

extern void gSolidify(const qreal widthT,
                      const SkPath &src,
                      SkPath * const dst);

#endif // PATHOPERATIONS_H
