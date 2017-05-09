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
    void setCurrentPointValue(const QPointF &val,
                              const bool &finish = false);
    void incCurrentValues(const qreal &x,
                          const qreal &y);
    void multCurrentValues(qreal sx, qreal sy);

    QPointF getSavedPointValue();
    qreal getSavedXValue();
    qreal getSavedYValue();

    void incAllValues(const qreal &x, const qreal &y);

    QrealAnimator *getXAnimator();
    QrealAnimator *getYAnimator();
    int prp_saveToSql(QSqlQuery *query, const int &parentId = 0);
    void prp_loadFromSql(const int &posAnimatorId);
    void makeDuplicate(Property *target);
    Property *makeDuplicate();

    void multSavedValueToCurrentValue(const qreal &sx,
                                      const qreal &sy);
    void incSavedValueToCurrentValue(const qreal &incXBy,
                                     const qreal &incYBy);

    void duplicateYAnimatorFrom(QrealAnimator *source);
    void duplicateXAnimatorFrom(QrealAnimator *source);
    void setValuesRange(const qreal &minVal,
                        const qreal &maxVal);
    QPointF getCurrentPointValueAtAbsFrame(const int &frame);
    QPointF getCurrentPointValueAtRelFrame(const int &frame);

    void setPrefferedValueStep(const qreal &valueStep);

    bool SWT_isQPointFAnimator() { return true; }
protected:
    QSharedPointer<QrealAnimator> mXAnimator =
            (new QrealAnimator())->ref<QrealAnimator>();
    QSharedPointer<QrealAnimator> mYAnimator =
            (new QrealAnimator())->ref<QrealAnimator>();
};

#endif // QPOINTFANIMATOR_H
