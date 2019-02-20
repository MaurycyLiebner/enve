#ifndef QCUBICSEGMENT1DANIMATOR_H
#define QCUBICSEGMENT1DANIMATOR_H
#include "Animators/graphanimatort.h"
#include "Segments/qcubicsegment1d.h"
#include "basicreadwrite.h"
class qCubicSegment1DAnimator;

class qCubicSegment1DAnimator : public GraphAnimatorT<qCubicSegment1D> {
    friend class SelfRef;
public:
    bool SWT_isQCubicSegment1DAnimator() const { return true; }
protected:
    qCubicSegment1DAnimator(const QString &name);
};

#endif // QCUBICSEGMENT1DANIMATOR_H
