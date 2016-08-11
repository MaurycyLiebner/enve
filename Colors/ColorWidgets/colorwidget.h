#ifndef COLORWIDGET_H
#define COLORWIDGET_H
#include <QOpenGLWidget>
#include "Colors/color.h"

extern GLfloat AA_VECT_LEN;
extern GLfloat AA_SHARP_VECT_LEN;

extern float clamp(float val_t, float min_t, float max_t);
extern float getAngleF(double x1, double y1, double x2, double y2);
extern bool shouldValPointerBeLightHSV(GLfloat hue, GLfloat saturation, GLfloat value);
extern bool shouldValPointerBeLightHSL(GLfloat hue, GLfloat saturation, GLfloat lightness);
extern bool shouldValPointerBeLightRGB(GLfloat r, GLfloat g, GLfloat b);

class ColorWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    bool isVisible();
    bool isHidden();
    void setVisible(bool b_t);
    void show();
    void hide();
    ColorWidget(QWidget *parent = NULL);
    void emitColorChangedSignal(bool external_signal_t = true);
protected:
    Color bgColor;
    GLfloat hsl_saturaton_tmp = -1.f; // HSL saturation saved when changing lightness

    GLfloat hue = 0.f;
    GLfloat saturation = 0.f;
    GLfloat value = 0.f;
    void normalize(GLfloat *x_t, GLfloat *y_t, GLfloat dest_len);
    void getAATrisVert(GLfloat center_x, GLfloat center_y, GLfloat x_t, GLfloat y_t,
                       GLfloat *result_x, GLfloat *result_y, GLfloat blurriness);
    void drawSolidAATris(GLfloat x1, GLfloat y1,
                         GLfloat x2, GLfloat y2,
                         GLfloat x3, GLfloat y3,
                         GLfloat r, GLfloat g, GLfloat b,
                         bool e12_aa, bool e13_aa, bool e23_aa);
    void drawAATris(GLfloat x1, GLfloat y1, GLfloat r1, GLfloat g1, GLfloat b1,
                    GLfloat x2, GLfloat y2, GLfloat r2, GLfloat g2, GLfloat b2,
                    GLfloat x3, GLfloat y3, GLfloat r3, GLfloat g3, GLfloat b3,
                    bool e12_aa, bool e13_aa, bool e23_aa);
    void drawAACircTris(GLfloat x1, GLfloat y1,
                        GLfloat x2, GLfloat y2,
                        GLfloat cx, GLfloat cy,
                        GLfloat r1, GLfloat g1, GLfloat b1,
                        GLfloat r2, GLfloat g2, GLfloat b2);
    void drawSubTris(GLfloat x1, GLfloat y1, GLfloat h1, GLfloat s1, GLfloat v1,
                     GLfloat x2, GLfloat y2, GLfloat h2, GLfloat s2, GLfloat v2,
                     GLfloat x3, GLfloat y3, GLfloat h3, GLfloat s3, GLfloat v3,
                     int n_next, bool e12_aa, bool e13_aa, bool e23_aa);
    void getAAVect(GLfloat center_x, GLfloat center_y, GLfloat x_t, GLfloat y_t,
                   GLfloat *result_x, GLfloat *result_y, GLfloat blurrines);
    void drawSolidCircle(GLfloat r, GLfloat cx, GLfloat cy, GLuint num_seg, GLfloat red, GLfloat green, GLfloat blue);
    void drawRect(GLfloat x, GLfloat y, GLfloat width, GLfloat height,
                  GLfloat r1, GLfloat g1, GLfloat b1,
                  GLfloat r2, GLfloat g2, GLfloat b2,
                  GLfloat r3, GLfloat g3, GLfloat b3,
                  GLfloat r4, GLfloat g4, GLfloat b4,
                  bool top_aa = true, bool bottom_aa = true, bool left_aa = true, bool right_aa = true);
    void drawSolidRect(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLfloat r, GLfloat g, GLfloat b,
                  bool top_aa = true, bool bottom_aa = true, bool left_aa = true, bool right_aa = true);
    void drawSolidRectCenter(GLfloat cx, GLfloat cy, GLfloat width, GLfloat height, GLfloat r, GLfloat g, GLfloat b,
                             bool top_aa = true, bool bottom_aa = true, bool left_aa = true, bool right_aa = true);

    void resizeGL(int w, int h);

    void setInternalColorChangeFunc(void (*internalColorChangeFunc_t)(ColorWidget *));
private:
    bool visible = true;
    void initializeGL();
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
