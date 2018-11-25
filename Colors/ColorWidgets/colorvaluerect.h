#ifndef COLORVALUERECT_H
#define COLORVALUERECT_H
#include "colorwidget.h"

enum CVR_TYPE : short {
    CVR_RED,
    CVR_GREEN,
    CVR_BLUE,

    CVR_HUE,
    CVR_HSVSATURATION,
    CVR_VALUE,

    CVR_HSLSATURATION,
    CVR_LIGHTNESS,
    CVR_ALPHA,
    CVR_ALL
};

class ColorValueRect : public ColorWidget {
    Q_OBJECT
public:
    ColorValueRect(const CVR_TYPE &type_t, QWidget *parent = nullptr);
    void updateVal();
    void setDisplayedValue(const qreal &val_t);
    qreal qVal() const {
        return static_cast<qreal>(mVal);
    }
private:
    void paintGL();
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *);
    //void wheelEvent(QWheelEvent *e);

    void mouseInteraction(const int &x_t);
    void setColorParameterFromVal();
    void setValueAndEmitValueChanged(const qreal &valT);
signals:
    void valChanged(const qreal&);
    void editingFinished(const qreal&);
    void editingStarted(const qreal&);
private:
    GLushort mNumberSegments = 5;
    CVR_TYPE mType;
    GLfloat mVal = 0.f;
};

#endif // COLORVALUERECT_H
