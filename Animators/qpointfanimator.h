#ifndef QPOINTFANIMATOR_H
#define QPOINTFANIMATOR_H
#include "Animators/complexanimator.h"
#include "Animators/qrealanimator.h"

class QPointFAnimator : public ComplexAnimator
{
public:
    QPointFAnimator();
    QPointF getCurrentValue() const;
    qreal getXValue();
    qreal getYValue();
    void setCurrentValue(QPointF val, bool finish = false);
    void incCurrentValue(qreal x, qreal y);
    void multCurrentValue(qreal sx, qreal sy);

    QPointF getSavedValue();
    qreal getSavedXValue();
    qreal getSavedYValue();

    void incAllValues(qreal x, qreal y);

    QrealAnimator *getXAnimator();
    QrealAnimator *getYAnimator();
    int saveToSql();
    void loadFromSql(int posAnimatorId);
    void multSavedValueToCurrentValue(qreal sx, qreal sy);
    void incSavedValueToCurrentValue(qreal incXBy, qreal incYBy);
private:
    QrealAnimator mXAnimator;
    QrealAnimator mYAnimator;
};

#endif // QPOINTFANIMATOR_H
