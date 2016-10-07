#ifndef COLORVALUESPIN_H
#define COLORVALUESPIN_H

#include <QObject>
#include <QDoubleSpinBox>
#include <GL/gl.h>

class ColorValueSpin : public QObject
{
    Q_OBJECT
public:
    ColorValueSpin(int min_val_t, int max_val_t, int val_t, QWidget *parent = 0);
    ~ColorValueSpin();
    QSpinBox *getSpinBox();
signals:
    void valSet(GLfloat);
public slots:
    void setVal(GLfloat val_t);
private slots:
    void spinBoxValSet(int spin_box_val_t);
private:
    bool mEmit = true;
    bool mBlockValue = false;
    QSpinBox *spin_box = NULL;
};

#endif // COLORVALUESPIN_H
