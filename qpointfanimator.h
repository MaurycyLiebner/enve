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
    void startTransform();
    void finishTransform(bool record);
    void retrieveSavedValue();

    QPointF getSavedValue();
    qreal getSavedXValue();
    qreal getSavedYValue();
    void cancelTransform();
    void setConnectedToMainWindow(ConnectedToMainWindow *connected);

    void setUpdater(AnimatorUpdater *updater);

    void setFrame(int frame);
private:
    QrealAnimator mXAnimator;
    QrealAnimator mYAnimator;
};

#endif // QPOINTFANIMATOR_H
