#ifndef COLORLABEL_H
#define COLORLABEL_H

#include "colorwidget.h"

class ColorLabel : public ColorWidget
{
    Q_OBJECT
public:
    explicit ColorLabel(QWidget *parent = 0);
    void saveCurrentColorAsLast();
    void setLastColorHSV(GLfloat h, GLfloat s, GLfloat v);
    void mousePressEvent(QMouseEvent *e);
signals:

public slots:
    void setAlpha(GLfloat alpha_t);
private:
    void paintGL();
    GLfloat alpha = 1.f;
};

#endif // COLORLABEL_H
