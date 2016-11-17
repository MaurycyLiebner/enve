#ifndef QREALANIMATORVALUESLIDER_H
#define QREALANIMATORVALUESLIDER_H
#include "qdoubleslider.h"

class QrealAnimator;

class QrealAnimatorValueSlider : public QDoubleSlider
{
public:
    QrealAnimatorValueSlider(qreal minVal, qreal maxVal,
                             QrealAnimator *animator, QWidget *parent = 0);

    void paint(QPainter *p);
protected:
    void emitValueChanged(qreal value);
    void emitEditingFinished(qreal value);
private:
    QrealAnimator *mAnimator;
};

#endif // QREALANIMATORVALUESLIDER_H
