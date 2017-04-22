#ifndef QPOINTFANIMATOR_H
#define QPOINTFANIMATOR_H
#include "Animators/complexanimator.h"
#include "Animators/qrealanimator.h"

class QPointFAnimator : public ComplexAnimator
{
public:
    QPointFAnimator();
    QPointF getCurrentPointValue() const;
    QPointF getPointValueAtFrame(const int &frame);
    qreal getXValue();
    qreal getYValue();
    void setCurrentPointValue(QPointF val, bool finish = false);
    void incCurrentValues(qreal x, qreal y);
    void multCurrentValues(qreal sx, qreal sy);

    QPointF getSavedPointValue();
    qreal getSavedXValue();
    qreal getSavedYValue();

    void incAllValues(qreal x, qreal y);

    QrealAnimator *getXAnimator();
    QrealAnimator *getYAnimator();
    int prp_saveToSql(QSqlQuery *query, const int &parentId = 0);
    void prp_loadFromSql(const int &posAnimatorId);
    void prp_makeDuplicate(Property *target);
    Property *prp_makeDuplicate();

    void multSavedValueToCurrentValue(qreal sx, qreal sy);
    void incSavedValueToCurrentValue(qreal incXBy, qreal incYBy);

    void duplicateYAnimatorFrom(QrealAnimator *source);
    void duplicateXAnimatorFrom(QrealAnimator *source);
    void setValuesRange(qreal minVal, qreal maxVal);
    QPointF getCurrentPointValueAtFrame(const int &frame);
    void setPrefferedValueStep(const qreal &valueStep);
protected:
    QSharedPointer<QrealAnimator> mXAnimator =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mYAnimator =
            (new QrealAnimator())->ref<QrealAnimator>();
};

#endif // QPOINTFANIMATOR_H
