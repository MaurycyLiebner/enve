#ifndef COLORVALUERECT_H
#define COLORVALUERECT_H
#include "colorwidget.h"

enum CVR_TYPE
{
    CVR_RED,
    CVR_GREEN,
    CVR_BLUE,

    CVR_HUE,
    CVR_HSVSATURATION,
    CVR_VALUE,

    CVR_HSLSATURATION,
    CVR_LIGHTNESS,
    CVR_ALPHA
};

class ColorValueRect : public ColorWidget
{
    Q_OBJECT
public:
    ColorValueRect(CVR_TYPE type_t, QWidget *parent = NULL);
    void updateVal();
    GLfloat getVal();
private:
    void paintGL();
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *e);

    void mouseInteraction(int x_t);
    void setColorParameterFromVal();
public slots:
    void setVal(GLfloat val_t, bool emit_signal = false);
    void setValAndEmitSignal(GLfloat val_t);
signals:
    void valChanged(GLfloat);
    void valUpdated(GLfloat);
private:
    CVR_TYPE type;
    GLfloat val = 0.f;
    GLushort number_segments = 5;
};

#endif // COLORVALUERECT_H
