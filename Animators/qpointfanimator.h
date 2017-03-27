#ifndef QPOINTFANIMATOR_H
#define QPOINTFANIMATOR_H
#include "Animators/complexanimator.h"
#include "Animators/qrealanimator.h"

class QPointFAnimator : public ComplexAnimator
{
public:
    QPointFAnimator();
    QPointF qra_getCurrentValue() const;
    QPointF getPointValueAtFrame(const int &frame);
    qreal getXValue();
    qreal getYValue();
    void qra_setCurrentValue(QPointF val, bool finish = false);
    void qra_incCurrentValue(qreal x, qreal y);
    void qra_multCurrentValue(qreal sx, qreal sy);

    QPointF qra_getSavedValue();
    qreal getSavedXValue();
    qreal getSavedYValue();

    void qra_incAllValues(qreal x, qreal y);

    QrealAnimator *getXAnimator();
    QrealAnimator *getYAnimator();
    int prp_saveToSql(QSqlQuery *query, const int &parentId);
    void prp_loadFromSql(const int &posAnimatorId);
    void prp_makeDuplicate(Property *target);
    Property *prp_makeDuplicate();

    void multSavedValueToCurrentValue(qreal sx, qreal sy);
    void incSavedValueToCurrentValue(qreal incXBy, qreal incYBy);

    void duplicateYAnimatorFrom(QrealAnimator *source);
    void duplicateXAnimatorFrom(QrealAnimator *source);
    void qra_setValueRange(qreal minVal, qreal maxVal);
    QPointF getCurrentPointValueAtFrame(const int &frame);
    void setPrefferedValueStep(const qreal &valueStep);
private:
    QrealAnimator mXAnimator;
    QrealAnimator mYAnimator;
};

#endif // QPOINTFANIMATOR_H
