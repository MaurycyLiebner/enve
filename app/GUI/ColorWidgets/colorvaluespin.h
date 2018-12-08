#ifndef COLORVALUESPIN_H
#define COLORVALUESPIN_H

#include <QObject>
#include <QDoubleSpinBox>
#include <QOpenGLFunctions_3_3_Core>
class QrealAnimatorValueSlider;

class ColorValueSpin : public QObject {
    Q_OBJECT
public:
    ColorValueSpin(QWidget *parent = nullptr);
    ~ColorValueSpin();
    QrealAnimatorValueSlider *getSpinBox();
signals:
    void valSet(GLfloat);
    void displayedValueChanged(GLfloat);
public slots:
    void setDisplayedValue(GLfloat val_t);
    void setValueExternal(const GLfloat &val);
private slots:
    void spinBoxDisplayedValueChanged(const qreal &val);
    void spinBoxValSet(double spin_box_val_t);
private:
    bool mBlockValue = false;
    QrealAnimatorValueSlider *mSpinBox = nullptr;
};

#endif // COLORVALUESPIN_H
