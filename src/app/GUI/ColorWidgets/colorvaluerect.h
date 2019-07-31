#ifndef COLORVALUERECT_H
#define COLORVALUERECT_H
#include "colorwidget.h"
#include "colorwidgetshaders.h"

class ColorValueRect : public ColorWidget {
    Q_OBJECT
public:
    ColorValueRect(const ColorProgram &program,
                   QWidget * const parent = nullptr);

    void setDisplayedValue(const qreal val_t);
    qreal value() const { return mVal; }
private:
    void paintGL();
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *);
    //void wheelEvent(QWheelEvent *e);

    void mouseInteraction(const int x_t);
    void setColorParameterFromVal();
    void setValueAndEmitValueChanged(const qreal valT);
signals:
    void valueChanged(qreal);
    void editingFinished(qreal);
    void editingStarted(qreal);
private:
    const ColorProgram& mGLProgram;
    qreal mVal = 0;
};

#endif // COLORVALUERECT_H
