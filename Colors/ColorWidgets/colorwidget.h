#ifndef COLORWIDGET_H
#define COLORWIDGET_H
#include "glwidget.h"
#include "Colors/color.h"

extern float clamp(float val_t, float min_t, float max_t);
extern float getAngleF(double x1, double y1, double x2, double y2);
extern bool shouldValPointerBeLightHSV(GLfloat hue, GLfloat saturation, GLfloat value);
extern bool shouldValPointerBeLightHSL(GLfloat hue, GLfloat saturation, GLfloat lightness);
extern bool shouldValPointerBeLightRGB(GLfloat r, GLfloat g, GLfloat b);

class ColorWidget : public GLWidget
{
    Q_OBJECT
public:
    ColorWidget(QWidget *parent = NULL);
protected:
    bool mValueBlocked = false;

    Color bgColor;
    GLfloat hsl_saturaton_tmp = -1.f; // HSL saturation saved when changing lightness

    GLfloat hue = 0.f;
    GLfloat saturation = 0.f;
    GLfloat value = 0.f;
public slots:
    void setColorHSV_f(GLfloat h, GLfloat s, GLfloat v);
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
