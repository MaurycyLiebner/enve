#ifndef QPOINTFANIMATOR_H
#define QPOINTFANIMATOR_H
#include "Animators/complexanimator.h"
#include "skia/skiaincludes.h"

class QPointFAnimator : public ComplexAnimator {
    friend class SelfRef;
protected:
    QPointFAnimator(const QString& name);
public:
    bool SWT_isQPointFAnimator() const { return true; }

    void setBaseValue(const qreal valX, const qreal valY) {
        setBaseValue({valX, valY});
    }
    void setBaseValue(const QPointF &val);

    void setBaseValueWithoutCallingUpdater(const QPointF &val);
    void incBaseValuesWithoutCallingUpdater(const qreal x, const qreal y);
    void incBaseValues(const qreal x, const qreal y);
    void multCurrentValues(const qreal sx, const qreal sy);

    QPointF getSavedValue();
    qreal getSavedXValue();
    qreal getSavedYValue();

    void incAllBaseValues(const qreal x, const qreal y);

    QrealAnimator *getXAnimator();
    QrealAnimator *getYAnimator();

    void multSavedValueToCurrentValue(const qreal sx,
                                      const qreal sy);
    void incSavedValueToCurrentValue(const qreal incXBy,
                                     const qreal incYBy);

    void setValuesRange(const qreal minVal, const qreal maxVal);

    QPointF getBaseValue() const;
    QPointF getBaseValueAtAbsFrame(const qreal frame) const;
    QPointF getBaseValueAtRelFrame(const qreal frame) const;

    void setPrefferedValueStep(const qreal valueStep);

    bool getBeingTransformed();

    QPointF getEffectiveValue() const;
    QPointF getEffectiveValueAtAbsFrame(const qreal frame) const;
    QPointF getEffectiveValueAtRelFrame(const qreal frame) const;

    qreal getEffectiveXValue();
    qreal getEffectiveXValue(const qreal relFrame);

    qreal getEffectiveYValue();
    qreal getEffectiveYValue(const qreal relFrame);
protected:
    qsptr<QrealAnimator> mXAnimator;
    qsptr<QrealAnimator> mYAnimator;
};

#endif // QPOINTFANIMATOR_H
