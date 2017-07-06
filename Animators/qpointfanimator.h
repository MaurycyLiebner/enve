#ifndef QPOINTFANIMATOR_H
#define QPOINTFANIMATOR_H
#include "Animators/complexanimator.h"
#include "skiaincludes.h"
class QrealAnimator;
typedef QSharedPointer<QrealAnimator> QrealAnimatorQSPtr;

class QPointFAnimator : public ComplexAnimator
{
public:
    QPointFAnimator();
    qreal getXValue();
    qreal getYValue();

    qreal getXValueAtRelFrame(const int &relFrame);
    qreal getYValueAtRelFrame(const int &relFrame);

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
    QPointF getCurrentPointValue() const;
    QPointF getCurrentPointValueAtAbsFrame(const int &frame) const;
    QPointF getCurrentPointValueAtRelFrame(const int &frame) const;

    void setPrefferedValueStep(const qreal &valueStep);

    bool SWT_isQPointFAnimator() { return true; }
protected:
    QrealAnimatorQSPtr mXAnimator;
    QrealAnimatorQSPtr mYAnimator;
};

#endif // QPOINTFANIMATOR_H
