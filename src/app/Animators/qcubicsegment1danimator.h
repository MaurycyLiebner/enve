#ifndef QCUBICSEGMENT1DANIMATOR_H
#define QCUBICSEGMENT1DANIMATOR_H
#include "Animators/interpolationanimatort.h"
#include "Segments/qcubicsegment1d.h"
#include "basicreadwrite.h"
class qCubicSegment1DAnimator;

class qCubicSegment1DAnimator : public InterpolationAnimatorT<qCubicSegment1D> {
    e_OBJECT
    Q_OBJECT
public:
    qCubicSegment1DAnimator(const QString &name);

    bool SWT_isQCubicSegment1DAnimator() const { return true; }
signals:
    void currentValueChanged(qCubicSegment1D);
protected:
    void afterValueChanged() {
        emit currentValueChanged(mCurrentValue);
    }
};

#endif // QCUBICSEGMENT1DANIMATOR_H
