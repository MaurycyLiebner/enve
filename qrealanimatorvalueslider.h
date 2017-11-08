#ifndef QREALANIMATORVALUESLIDER_H
#define QREALANIMATORVALUESLIDER_H
#include "qdoubleslider.h"

class IntProperty;
class QrealAnimator;
class Property;

class QrealAnimatorValueSlider : public QDoubleSlider
{
    Q_OBJECT
public:
    QrealAnimatorValueSlider(QString name, qreal minVal,
                             qreal maxVal, qreal prefferedStep,
                             QWidget *parent);
    QrealAnimatorValueSlider(qreal minVal, qreal maxVal, qreal prefferedStep,
                             QWidget *parent);
    QrealAnimatorValueSlider(qreal minVal, qreal maxVal, qreal prefferedStep,
                             QrealAnimator *animator, QWidget *parent = 0);
    QrealAnimatorValueSlider(QrealAnimator *animator, QWidget *parent = 0);
    QrealAnimatorValueSlider(QWidget *parent = 0);
    ~QrealAnimatorValueSlider() {}

    void paint(QPainter *p);

    void setAnimator(QrealAnimator *animator);
    void setIntAnimator(IntProperty *animator);

    bool hasTargetAnimator() {
        return mAnimator != NULL;
    }
    void openContextMenu(const QPoint &globalPos);
    void clearAnimator();
public slots:
    void setValueExternal(qreal value);

    void emitEditingStarted(qreal value);
    void emitValueChangedExternal(qreal value);
    void emitEditingFinished(qreal value);

    void nullifyAnimator();
    void setValueFromAnimator(qreal val);
protected:
    void emitValueChanged(qreal value);
private:
    Property *mAnimator = NULL;
    bool mBlockAnimatorSignals = false;
signals:
    void displayedValueChanged(qreal);
};

#endif // QREALANIMATORVALUESLIDER_H
