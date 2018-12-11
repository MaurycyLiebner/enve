#ifndef COLORWIDGET_H
#define COLORWIDGET_H
#include "glwidget.h"
#include "GUI/ColorWidgets/helpers.h"

class ColorWidget : public GLWidget {
    Q_OBJECT
public:
    ColorWidget(QWidget *parent = nullptr);

    void setColorHSV(const qreal& h,
                     const qreal& s,
                     const qreal& v) {
        setColorHSV_f(static_cast<GLfloat>(h),
                      static_cast<GLfloat>(s),
                      static_cast<GLfloat>(v));
    }
    void setColorHSV_f(GLfloat h, GLfloat s, GLfloat v);
protected:
    bool mValueBlocked = false;

    GLfloat mHslSaturatonTmp = -1.f; // HSL saturation saved when changing lightness

    GLfloat mHue = 0.f;
    GLfloat mSaturation = 0.f;
    GLfloat mValue = 0.f;
public slots:
    void setHue_f(GLfloat h);
    void setHSVSaturation_f(GLfloat s);
    void setValue_f(GLfloat v);

    void setColorHSV_i(GLushort h, GLushort s, GLushort v);
    void setHue_i(GLushort h);
    void setHSVSaturation_i(GLushort s);
    void setValue_i(GLushort v);

    void setColorHSL_f(GLfloat h, GLfloat s, GLfloat l);
    void setLightness_f(GLfloat l);
    void setHSLSaturation_f(GLfloat s);

    void setColorHSL_i(GLushort h, GLushort s, GLushort l);
    void setLightness_i(GLushort l);
    void setHSLSaturation_i(GLushort s);

    void setColorRGB_f(GLfloat r, GLfloat g, GLfloat b);
    void setR_f(GLfloat r);
    void setG_f(GLfloat g);
    void setB_f(GLfloat b);

    void setR_i(GLushort r);
    void setG_i(GLushort g);
    void setB_i(GLushort b);
};

#endif // COLORWIDGET_H
