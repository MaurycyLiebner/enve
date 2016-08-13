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
    void emitColorChangedSignal(bool external_signal_t = true);
protected:
    Color bgColor;
    GLfloat hsl_saturaton_tmp = -1.f; // HSL saturation saved when changing lightness

    GLfloat hue = 0.f;
    GLfloat saturation = 0.f;
    GLfloat value = 0.f;

    void setInternalColorChangeFunc(void (*internalColorChangeFunc_t)(ColorWidget *));

private:
    void (*internalColorChangeFunc)(ColorWidget *) = NULL;
signals:
    void colorChangedHSV(GLfloat h, GLfloat s, GLfloat v);
public slots:
    void setColorHSV_f(GLfloat h, GLfloat s, GLfloat v, bool emit_signal = false);
    void setHue_f(GLfloat h, bool emit_signal = false);
    void setHSVSaturation_f(GLfloat s, bool emit_signal = false);
    void setValue_f(GLfloat v, bool emit_signal = false);

    void setColorHSV_i(GLushort h, GLushort s, GLushort v, bool emit_signal = false);
    void setHue_i(GLushort h, bool emit_signal = false);
    void setHSVSaturation_i(GLushort s, bool emit_signal = false);
    void setValue_i(GLushort v, bool emit_signal = false);

    void setColorHSL_f(GLfloat h, GLfloat s, GLfloat l, bool emit_signal = false);
    void setLightness_f(GLfloat l, bool emit_signal = false);
    void setHSLSaturation_f(GLfloat s, bool emit_signal = false);

    void setColorHSL_i(GLushort h, GLushort s, GLushort l, bool emit_signal = false);
    void setLightness_i(GLushort l, bool emit_signal = false);
    void setHSLSaturation_i(GLushort s, bool emit_signal = false);

    void setColorRGB_f(GLfloat r, GLfloat g, GLfloat b, bool emit_signal = false);
    void setR_f(GLfloat r, bool emit_signal = false);
    void setG_f(GLfloat g, bool emit_signal = false);
    void setB_f(GLfloat b, bool emit_signal = false);

    void setR_i(GLushort r, bool emit_signal = false);
    void setG_i(GLushort g, bool emit_signal = false);
    void setB_i(GLushort b, bool emit_signal = false);
};

#endif // COLORWIDGET_H
