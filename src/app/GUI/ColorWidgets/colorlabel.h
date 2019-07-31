#ifndef COLORLABEL_H
#define COLORLABEL_H

#include "colorwidget.h"


class ColorLabel : public ColorWidget {
    Q_OBJECT
public:
    explicit ColorLabel(QWidget *parent = nullptr);
    void saveCurrentColorAsLast();
    void setLastColorHSV(GLfloat h, GLfloat s, GLfloat v);
    void mousePressEvent(QMouseEvent *e);
signals:

public slots:
    void setAlpha(const qreal alpha_t);
private:
    void paintGL();
    qreal mAlpha = 1;
};

#endif // COLORLABEL_H
