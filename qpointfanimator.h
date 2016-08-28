#ifndef QPOINTFANIMATOR_H
#define QPOINTFANIMATOR_H
#include "complexanimator.h"
#include "qrealanimator.h"

class QPointFAnimator : public ComplexAnimator
{
public:
    QPointFAnimator();
    QPointF getCurrentValue();
    qreal getXValue();
    qreal getYValue();
    void setCurrentValue(QPointF val);
    void incCurrentValue(qreal x, qreal y);
    void multCurrentValue(qreal sx, qreal sy);
    void saveCurrentValue();
    void retrieveSavedValue();

    QPointF getSavedValue();
    qreal getSavedXValue();
    qreal getSavedYValue();
private:
    QrealAnimator mXAnimator;
    QrealAnimator mYAnimator;
};

#endif // QPOINTFANIMATOR_H
