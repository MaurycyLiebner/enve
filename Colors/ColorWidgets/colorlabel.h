#ifndef COLORLABEL_H
#define COLORLABEL_H

#include "colorwidget.h"

class ColorLabel : public ColorWidget
{
    Q_OBJECT
public:
    explicit ColorLabel(GLfloat h_t, GLfloat s_t, GLfloat v_t, QWidget *parent = 0);
    void saveCurrentColorAsLast();
    void setLastColorHSV(GLfloat h, GLfloat s, GLfloat v);
    void mousePressEvent(QMouseEvent *e);
signals:

public slots:
private:
    void paintGL();
    GLfloat last_color_r = 0.f;
    GLfloat last_color_g = 0.f;
    GLfloat last_color_b = 0.f;
    GLfloat last_color_h = 0.f;
    GLfloat last_color_s = 0.f;
    GLfloat last_color_v = 0.f;
};

#endif // COLORLABEL_H
