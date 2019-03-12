#ifndef QCUBICSEGMENT1DANIMATOR_H
#define QCUBICSEGMENT1DANIMATOR_H
#include "Animators/interpolationanimatort.h"
#include "Segments/qcubicsegment1d.h"
#include "basicreadwrite.h"
class qCubicSegment1DAnimator;

class qCubicSegment1DAnimator : public InterpolationAnimatorT<qCubicSegment1D> {
    friend class SelfRef;
    Q_OBJECT
public:
    bool SWT_isQCubicSegment1DAnimator() const { return true; }
signals:
    void currentValueChanged(qCubicSegment1D);
protected:
    qCubicSegment1DAnimator(const QString &name);
    void afterValueChanged() {
        emit currentValueChanged(mCurrentValue);
    }
};

#endif // QCUBICSEGMENT1DANIMATOR_H
