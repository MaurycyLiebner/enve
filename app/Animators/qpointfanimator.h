#ifndef QPOINTFANIMATOR_H
#define QPOINTFANIMATOR_H
#include "Animators/complexanimator.h"
#include "skia/skiaincludes.h"

class QPointFAnimator : public ComplexAnimator {
    friend class SelfRef;
public:
    qreal getXValue();
    qreal getYValue();

    qreal getXValueAtRelFrame(const qreal &relFrame);
    qreal getYValueAtRelFrame(const qreal &relFrame);

    void setCurrentPointValue(const QPointF &val);
    void incCurrentValues(const qreal &x, const qreal &y);
    void multCurrentValues(qreal sx, qreal sy);

    QPointF getSavedPointValue();
    qreal getSavedXValue();
    qreal getSavedYValue();

    void incAllValues(const qreal &x, const qreal &y);

    QrealAnimator *getXAnimator();
    QrealAnimator *getYAnimator();

    void multSavedValueToCurrentValue(const qreal &sx,
                                      const qreal &sy);
    void incSavedValueToCurrentValue(const qreal &incXBy,
                                     const qreal &incYBy);

    void setValuesRange(const qreal &minVal,
                        const qreal &maxVal);
    QPointF getCurrentPointValue() const;
    QPointF getCurrentPointValueAtAbsFrameF(const qreal &frame) const;
    QPointF getCurrentPointValueAtRelFrame(const qreal &frame) const;

    void setPrefferedValueStep(const qreal &valueStep);

    bool SWT_isQPointFAnimator() const { return true; }
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);

    bool getBeingTransformed();
    QPointF getCurrentEffectivePointValueAtAbsFrameF(const qreal &frame) const;
    QPointF getCurrentEffectivePointValueAtRelFrame(const qreal &frame) const;
    QPointF getCurrentEffectivePointValue() const;
    qreal getEffectiveXValueAtRelFrame(const qreal &relFrame);
    qreal getEffectiveYValueAtRelFrame(const qreal &relFrame);
    qreal getEffectiveXValue();
    qreal getEffectiveYValue();
protected:
    QPointFAnimator(const QString& name);

    qsptr<QrealAnimator> mXAnimator;
    qsptr<QrealAnimator> mYAnimator;
};

#endif // QPOINTFANIMATOR_H
