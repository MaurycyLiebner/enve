#ifndef QREALANIMATORVALUESLIDER_H
#define QREALANIMATORVALUESLIDER_H
#include "qdoubleslider.h"

class QrealAnimator;

class QrealAnimatorValueSlider : public QDoubleSlider
{
    Q_OBJECT
public:
    QrealAnimatorValueSlider(qreal minVal, qreal maxVal, qreal prefferedStep,
                             QWidget *parent);
    QrealAnimatorValueSlider(qreal minVal, qreal maxVal, qreal prefferedStep,
                             QrealAnimator *animator, QWidget *parent = 0);
    QrealAnimatorValueSlider(QrealAnimator *animator, QWidget *parent = 0);
    ~QrealAnimatorValueSlider();

    void paint(QPainter *p);

    void setAnimator(QrealAnimator *animator);
protected:
    void emitValueChanged(qreal value);
    void emitEditingFinished(qreal value);
private:
    QrealAnimator *mAnimator = NULL;
};

#endif // QREALANIMATORVALUESLIDER_H
