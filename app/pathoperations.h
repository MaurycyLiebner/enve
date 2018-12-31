#ifndef PATHOPERATIONS_H
#define PATHOPERATIONS_H
#include <QPainterPath>
class PathBox;
class SkPath;
class PathAnimator;

extern void gApplyOperationF(const qreal &relFrame, const SkPath &src,
                             SkPath *dst, PathBox *srcBox,
                             PathBox *dstBox, const QString &operation,
                             const bool &groupSum = false);

extern void gApplyOperation(const int &relFrame, const SkPath &src,
                            SkPath *dst, PathBox *srcBox,
                            PathBox *dstBox, const QString &operation,
                            const bool &groupSum = false);

extern void gSolidify(const qreal &widthT, const SkPath &src, SkPath *dst);

#endif // PATHOPERATIONS_H
