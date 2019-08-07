#ifndef QREALANIMATORVALUESLIDER_H
#define QREALANIMATORVALUESLIDER_H
#include "qdoubleslider.h"
#include "smartPointers/ememory.h"
class IntProperty;
class QrealAnimator;
class Property;

class QrealAnimatorValueSlider : public QDoubleSlider {
    Q_OBJECT
public:
    QrealAnimatorValueSlider(QString name, qreal minVal,
                             qreal maxVal, qreal prefferedStep,
                             QWidget *parent);
    QrealAnimatorValueSlider(qreal minVal, qreal maxVal, qreal prefferedStep,
                             QWidget *parent);
    QrealAnimatorValueSlider(qreal minVal, qreal maxVal, qreal prefferedStep,
                             QrealAnimator* animator, QWidget *parent = nullptr);
    QrealAnimatorValueSlider(QrealAnimator* animator, QWidget *parent = nullptr);
    QrealAnimatorValueSlider(QWidget *parent = nullptr);
    ~QrealAnimatorValueSlider() {}

    void paint(QPainter *p);
    void openContextMenu(const QPoint &globalPos);

    void setTarget(QrealAnimator * const animator);
    void setTarget(IntProperty * const animator);

    bool hasTarget();

    bool isTargetDisabled();

    void clearTarget();

    void setValueExternal(qreal value);

    void emitEditingStarted(qreal value);
    void emitValueChangedExternal(qreal value);
    void emitEditingFinished(qreal value);

    void nullifyAnimator();
    void setValueFromAnimator(qreal val);
protected:
    void emitValueChanged(qreal value);
    void mouseMoveEvent(QMouseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
private:
    QrealAnimator *getQPointFAnimatorSibling();

    bool mBlockAnimatorSignals = false;
    qptr<Property> mTarget;
signals:
    void displayedValueChanged(qreal);
};

#endif // QREALANIMATORVALUESLIDER_H
