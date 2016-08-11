#include "colorwidget.h"
#include "Colors/helpers.h"
#include "mainwindow.h"

GLfloat AA_VECT_LEN = 1.5f;
GLfloat AA_SHARP_VECT_LEN = AA_VECT_LEN*2.f;

bool shouldValPointerBeLightHSV(GLfloat hue, GLfloat saturation, GLfloat value)
{
    return value < 0.6f || (hue > 0.6f && (saturation > 0.5f || value < 0.7f) );
}

bool shouldValPointerBeLightHSL(GLfloat hue, GLfloat saturation, GLfloat lightness)
{
    hsl_to_hsv(&hue, &saturation, &lightness);
    return shouldValPointerBeLightHSV(hue, saturation, lightness);
}

bool shouldValPointerBeLightRGB(GLfloat r, GLfloat g, GLfloat b)
{
    rgb_to_hsv_float(&r, &g, &b);
    return shouldValPointerBeLightHSV(r, g, b);
}

bool ColorWidget::isVisible()
{
    return visible;
}

bool ColorWidget::isHidden()
{
    return !visible;
}

void ColorWidget::setVisible(bool b_t)
{
    visible = b_t;
    QWidget::setVisible(b_t);
}

void ColorWidget::show()
{
    visible = true;
    QWidget::show();
}

void ColorWidget::hide()
{
    visible = false;
    QWidget::hide();
}

ColorWidget::ColorWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    QColor col = QWidget::palette().color(QWidget::backgroundRole());
    bgColor = Color(col.red(), col.green(), col.blue());
    setMinimumSize(20, 20);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
}


void ColorWidget::setColorHSV_f(GLfloat h, GLfloat s, GLfloat v, bool emit_signal)
{
    hue = h;
    saturation = s;
    value = v;
    emitColorChangedSignal(emit_signal);
}

void ColorWidget::setHue_f(GLfloat h, bool emit_signal)
{
    hue = h;
    emitColorChangedSignal(emit_signal);
}

void ColorWidget::setHSVSaturation_f(GLfloat s, bool emit_signal)
{
    saturation = s;
    emitColorChangedSignal(emit_signal);
}

void ColorWidget::setValue_f(GLfloat v, bool emit_signal)
{
    value = v;
    emitColorChangedSignal(emit_signal);
}

void ColorWidget::setColorHSV_i(GLushort h, GLushort s, GLushort v, bool emit_signal)
{
    hue = clamp(h/360.f, 0.f, 1.f);
    saturation = clamp(s*0.01f, 0.f, 1.f);
    value = clamp(v*0.01f, 0.f, 1.f);
    emitColorChangedSignal(emit_signal);
}

void ColorWidget::setHue_i(GLushort h, bool emit_signal)
{
    hue = clamp(h/360.f, 0.f, 1.f);
    emitColorChangedSignal(emit_signal);
}

void ColorWidget::setHSVSaturation_i(GLushort s, bool emit_signal)
{
    saturation = clamp(s*0.01f, 0.f, 1.f);
    emitColorChangedSignal(emit_signal);
}

void ColorWidget::setValue_i(GLushort v, bool emit_signal)
{
    value = clamp(v*0.01f, 0.f, 1.f);
    emitColorChangedSignal(emit_signal);
}

void ColorWidget::setColorHSL_f(GLfloat h, GLfloat s, GLfloat l, bool emit_signal)
{
    hsl_to_hsv(&h, &s, &l);
    setColorHSV_f(h, s, l, emit_signal);
}

void ColorWidget::setLightness_f(GLfloat l, bool emit_signal)
{
    GLfloat hsv_h = hue;
    GLfloat hsv_s = saturation;
    GLfloat hsv_v = value;
    hsv_to_hsl(&hsv_h, &hsv_s, &hsv_v);
    if(hsl_saturaton_tmp > -0.5f)
    {
        hsv_s = hsl_saturaton_tmp;
    }
    else
    {
        hsl_saturaton_tmp = hsv_s;
    }
    hsl_to_hsv(&hsv_h, &hsv_s, &l);
    setColorHSV_f(hue, hsv_s, l, emit_signal);
}

void ColorWidget::setHSLSaturation_f(GLfloat s, bool emit_signal)
{
    GLfloat hsv_h = hue;
    GLfloat hsv_s = saturation;
    GLfloat hsv_v = value;
    hsv_to_hsl(&hsv_h, &hsv_s, &hsv_v);

    hsl_to_hsv(&hsv_h, &s, &hsv_v);

    setColorHSV_f(hue, s, hsv_v, emit_signal);
}

void ColorWidget::setColorHSL_i(GLushort h, GLushort s, GLushort l, bool emit_signal)
{
    GLfloat h_f = clamp(h/360.f, 0.f, 1.f);
    GLfloat s_f = clamp(s*0.01f, 0.f, 1.f);
    GLfloat l_f = clamp(l*0.01f, 0.f, 1.f);
    setColorHSL_f(h_f, s_f, l_f);
}

void ColorWidget::setLightness_i(GLushort l, bool emit_signal)
{
    GLfloat l_f = clamp(l*0.01f, 0.f, 1.f);
    setLightness_f(l_f, emit_signal);
}

void ColorWidget::setHSLSaturation_i(GLushort s, bool emit_signal)
{
    GLfloat s_f = clamp(s*0.01f, 0.f, 1.f);
    setHSLSaturation_f(s_f, emit_signal);
}

void ColorWidget::setColorRGB_f(GLfloat r, GLfloat g, GLfloat b, bool emit_signal)
{
    rgb_to_hsv_float(&r, &g, &b);
    hue = r;
    saturation = g;
    value = b;
    emitColorChangedSignal(emit_signal);
}

void ColorWidget::setR_f(GLfloat r, bool emit_signal)
{
    GLfloat c_r = hue;
    GLfloat c_g = saturation;
    GLfloat c_b = value;
    hsv_to_rgb_float(&c_r, &c_g, &c_b);
    setColorRGB_f(r, c_g, c_b, emit_signal);
}

void ColorWidget::setG_f(GLfloat g, bool emit_signal)
{
    GLfloat c_r = hue;
    GLfloat c_g = saturation;
    GLfloat c_b = value;
    hsv_to_rgb_float(&c_r, &c_g, &c_b);
    setColorRGB_f(c_r, g, c_b, emit_signal);
}

void ColorWidget::setB_f(GLfloat b, bool emit_signal)
{
    GLfloat c_r = hue;
    GLfloat c_g = saturation;
    GLfloat c_b = value;
    hsv_to_rgb_float(&c_r, &c_g, &c_b);
    setColorRGB_f(c_r, c_g, b, emit_signal);
}

void ColorWidget::setR_i(GLushort r, bool emit_signal)
{
    GLfloat r_f = r/255.f;
    setR_f(r_f, emit_signal);
}

void ColorWidget::setG_i(GLushort g, bool emit_signal)
{
    GLfloat g_f = g/255.f;
    setG_f(g_f, emit_signal);
}

void ColorWidget::setB_i(GLushort b, bool emit_signal)
{
    GLfloat b_f = b/255.f;
    setB_f(b_f, emit_signal);
}

void ColorWidget::emitColorChangedSignal(bool external_signal_t)
{
    update();
    if(external_signal_t)
    {
        emit colorChangedHSV(hue, saturation, value);
    }
    else if(internalColorChangeFunc != NULL)
    {
        internalColorChangeFunc(this);
    }
}

void ColorWidget::drawSubTris(GLfloat x1, GLfloat y1,
                              GLfloat h1, GLfloat s1, GLfloat v1,
                              GLfloat x2, GLfloat y2,
                              GLfloat h2, GLfloat s2, GLfloat v2,
                              GLfloat x3, GLfloat y3,
                              GLfloat h3, GLfloat s3, GLfloat v3,
                              int n_next, bool e12_aa, bool e13_aa, bool e23_aa)
{
    n_next--;
    if(n_next > 0)
    {
        GLfloat x12 = (x1 + x2)*0.5;
        GLfloat y12 = (y1 + y2)*0.5;

        GLfloat h12 = (h1 + h2)*0.5;
        GLfloat s12 = (s1 + s2)*0.5;
        GLfloat v12 = (v1 + v2)*0.5;

        GLfloat x13 = (x1 + x3)*0.5;
        GLfloat y13 = (y1 + y3)*0.5;

        GLfloat h13 = (h1 + h3)*0.5;
        GLfloat s13 = (s1 + s3)*0.5;
        GLfloat v13 = (v1 + v3)*0.5;

        GLfloat x23 = (x2 + x3)*0.5;
        GLfloat y23 = (y2 + y3)*0.5;

        GLfloat h23 = (h2 + h3)*0.5;
        GLfloat s23 = (s2 + s3)*0.5;
        GLfloat v23 = (v2 + v3)*0.5;
        drawSubTris(x1, y1,
                    h1, s1, v1,
                    x12, y12,
                    h12, s12, v12,
                    x13, y13,
                    h13, s13, v13,
                    n_next, e12_aa, e13_aa, false);
        drawSubTris(x12, y12,
                    h12, s12, v12,
                    x2, y2,
                    h2, s2, v2,
                    x23, y23,
                    h23, s23, v23,
                    n_next, e12_aa, false, e23_aa);
        drawSubTris(x13, y13,
                    h13, s13, v13,
                    x23, y23,
                    h23, s23, v23,
                    x3, y3,
                    h3, s3, v3,
                    n_next, false, e13_aa, e23_aa);
        drawSubTris(x12, y12,
                    h12, s12, v12,
                    x13, y13,
                    h13, s13, v13,
                    x23, y23,
                    h23, s23, v23,
                    n_next, false, false, false);
    }
    else
    {
        hsv_to_rgb_float(&h1, &s1, &v1);
        hsv_to_rgb_float(&h2, &s2, &v2);
        hsv_to_rgb_float(&h3, &s3, &v3);
        drawAATris(x1, y1,
                   h1, s1, v1,
                   x2, y2,
                   h2, s2, v2,
                   x3, y3,
                   h3, s3, v3,
                   e12_aa, e13_aa, e23_aa);
    }
}

void ColorWidget::normalize(GLfloat *x_t, GLfloat *y_t, GLfloat dest_len)
{
    GLfloat x_val_t = *x_t;
    GLfloat y_val_t = *y_t;
    GLfloat curr_len = sqrt(x_val_t*x_val_t + y_val_t*y_val_t);
    *x_t = x_val_t*dest_len/curr_len;
    *y_t = y_val_t*dest_len/curr_len;
}

void ColorWidget::getAAVect(GLfloat center_x, GLfloat center_y,
                                    GLfloat x_t, GLfloat y_t,
                                    GLfloat *result_x, GLfloat *result_y,
                                    GLfloat blurrines)
{
    GLfloat dx = x_t - center_x;
    GLfloat dy = y_t - center_y;
    normalize(&dx, &dy, blurrines);
    *result_x = dx;
    *result_y = dy;
}

void ColorWidget::getAATrisVert(GLfloat center_x, GLfloat center_y,
                                        GLfloat x_t, GLfloat y_t,
                                        GLfloat *result_x, GLfloat *result_y,
                                        GLfloat blurriness)
{
     GLfloat dx;
     GLfloat dy;
     getAAVect(center_x, center_y, x_t, y_t, &dx, &dy, blurriness);
     *result_x = x_t + dx;
     *result_y = y_t + dy;
}

void ColorWidget::drawSolidAATris(GLfloat x1, GLfloat y1,
                                  GLfloat x2, GLfloat y2,
                                  GLfloat x3, GLfloat y3,
                                  GLfloat r, GLfloat g, GLfloat b,
                                  bool e12_aa, bool e13_aa, bool e23_aa)
{
    drawAATris(x1, y1,
               r, g, b,
               x2, y2,
               r, g, b,
               x3, y3,
               r, g, b,
               e12_aa, e13_aa, e23_aa);
}

void ColorWidget::drawAATris(GLfloat x1, GLfloat y1,
                                     GLfloat r1, GLfloat g1, GLfloat b1,
                                     GLfloat x2, GLfloat y2,
                                     GLfloat r2, GLfloat g2, GLfloat b2,
                                     GLfloat x3, GLfloat y3,
                                     GLfloat r3, GLfloat g3, GLfloat b3,
                                     bool e12_aa, bool e13_aa, bool e23_aa)
{
    GLfloat center_x = (x1 + x2 + x3)/3;
    GLfloat center_y = (y1 + y2 + y3)/3;

    GLfloat x11_sharp;
    GLfloat y11_sharp;

    GLfloat x22_sharp;
    GLfloat y22_sharp;

    GLfloat x33_sharp;
    GLfloat y33_sharp;


    GLfloat x11;
    GLfloat y11;
    GLfloat x22;
    GLfloat y22;
    GLfloat x33;
    GLfloat y33;
    bool sharp_1 = e12_aa && e13_aa;
    bool sharp_2 = e12_aa && e23_aa;
    bool sharp_3 = e13_aa && e23_aa;
    if(sharp_1)
    {
        getAATrisVert(center_x, center_y, x1, y1, &x11_sharp, &y11_sharp, AA_SHARP_VECT_LEN);
        x11 = x11_sharp;
        y11 = y11_sharp;
    }
    if(sharp_2)
    {
        getAATrisVert(center_x, center_y, x2, y2, &x22_sharp, &y22_sharp, AA_SHARP_VECT_LEN);
        x22 = x22_sharp;
        y22 = y22_sharp;
    }
    if(sharp_3)
    {
        getAATrisVert(center_x, center_y, x3, y3, &x33_sharp, &y33_sharp, AA_SHARP_VECT_LEN);
        x33 = x33_sharp;
        y33 = y33_sharp;
    }
    glBegin(GL_TRIANGLES);
        glColor3f(r1, g1, b1);
        glVertex2f(x1, y1);
        glColor3f(r2, g2, b2);
        glVertex2f(x2, y2);
        glColor3f(r3, g3, b3);
        glVertex2f(x3, y3);

        if(e12_aa)
        {
            GLfloat dx12;
            GLfloat dy12;
            if(!sharp_1 || !sharp_2)
            {
                getAAVect(center_x, center_y, (x1 + x2)*0.5, (y1 + y2)*0.5, &dx12, &dy12, AA_VECT_LEN);
            }
            if(!sharp_1)
            {
                x11 = x1 + dx12;
                y11 = y1 + dy12;
            }
            glColor3f(r1, g1, b1);
            glVertex2f(x1, y1);
            glColor4f(r1, g1, b1, 0.f);
            glVertex2f(x11, y11);
            glColor3f(r2, g2, b2);
            glVertex2f(x2, y2);

            if(!sharp_2)
            {
                x22 = x2 + dx12;
                y22 = y2 + dy12;
            }
            glColor3f(r2, g2, b2);
            glVertex2f(x2, y2);
            glColor4f(r1, g1, b1, 0.f);
            glVertex2f(x11, y11);
            glColor4f(r2, g2, b2, 0.f);
            glVertex2f(x22, y22);
        }

        if(e13_aa)
        {
            GLfloat dx13;
            GLfloat dy13;
            if(!sharp_1 || !sharp_3)
            {
                getAAVect(center_x, center_y, (x1 + x3)*0.5, (y1 + y3)*0.5, &dx13, &dy13, AA_VECT_LEN);
            }
            if(!sharp_1)
            {
                x11 = x1 + dx13;
                y11 = y1 + dy13;
            }
            glColor3f(r1, g1, b1);
            glVertex2f(x1, y1);
            glColor4f(r1, g1, b1, 0.f);
            glVertex2f(x11, y11);
            glColor3f(r3, g3, b3);
            glVertex2f(x3, y3);

            if(!sharp_3)
            {
                x33 = x3 + dx13;
                y33 = y3 + dy13;
            }
            glColor3f(r3, g3, b3);
            glVertex2f(x3, y3);
            glColor4f(r1, g1, b1, 0.f);
            glVertex2f(x11, y11);
            glColor4f(r3, g3, b3, 0.f);
            glVertex2f(x33, y33);
        }

        if(e23_aa)
        {
            GLfloat dx23;
            GLfloat dy23;
            if(!sharp_2 || !sharp_3)
            {
                getAAVect(center_x, center_y, (x2 + x3)*0.5, (y2 + y3)*0.5, &dx23, &dy23, AA_VECT_LEN);
            }

            if(!sharp_3)
            {
                x33 = x3 + dx23;
                y33 = y3 + dy23;
            }
            glColor3f(r3, g3, b3);
            glVertex2f(x3, y3);
            glColor4f(r3, g3, b3, 0.f);
            glVertex2f(x33, y33);
            glColor3f(r2, g2, b2);
            glVertex2f(x2, y2);

            if(!sharp_2)
            {
                x22 = x2 + dx23;
                y22 = y2 + dy23;
            }
            glColor3f(r2, g2, b2);
            glVertex2f(x2, y2);
            glColor4f(r3, g3, b3, 0.f);
            glVertex2f(x33, y33);
            glColor4f(r2, g2, b2, 0.f);
            glVertex2f(x22, y22);
        }
    glEnd();
}

void ColorWidget::drawSolidRect(GLfloat x, GLfloat y, GLfloat width, GLfloat height,
                                GLfloat r, GLfloat g, GLfloat b,
                                bool top_aa, bool bottom_aa, bool left_aa, bool right_aa)
{
    drawRect(x, y, width, height,
             r, g, b,
             r, g, b,
             r, g, b,
             r, g, b,
             top_aa, bottom_aa, left_aa, right_aa);
}

void ColorWidget::drawSolidRect(GLfloat x, GLfloat y, GLfloat width, GLfloat height,
                                GLfloat r, GLfloat g, GLfloat b, GLfloat a,
                                bool top_aa, bool bottom_aa, bool left_aa, bool right_aa)
{
    drawRect(x, y, width, height,
             r, g, b, a,
             r, g, b, a,
             r, g, b, a,
             r, g, b, a,
             top_aa, bottom_aa, left_aa, right_aa);
}

void ColorWidget::drawSolidRectCenter(GLfloat cx, GLfloat cy, GLfloat width, GLfloat height,
                                         GLfloat r, GLfloat g, GLfloat b,
                                      bool top_aa, bool bottom_aa, bool left_aa, bool right_aa)
{
    drawSolidRect(cx - width*0.5f, cy - height*0.5f, width, height, r, g, b,
                  top_aa, bottom_aa, left_aa, right_aa);
}

void ColorWidget::resizeGL(int w, int h)
{
    glOrthoAndViewportSet(w, h);
}

void ColorWidget::setInternalColorChangeFunc(void (*internalColorChangeFunc_t)(ColorWidget *))
{
    internalColorChangeFunc = internalColorChangeFunc_t;
}

void ColorWidget::initializeGL()
{
    bgColor.setGLClearColor();

    //Set blending
    glEnable( GL_BLEND );
    glDisable( GL_DEPTH_TEST );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}


void ColorWidget::drawAACircTris(GLfloat x1, GLfloat y1,
                                 GLfloat x2, GLfloat y2,
                                 GLfloat cx, GLfloat cy,
                                 GLfloat r1, GLfloat g1, GLfloat b1,
                                 GLfloat r2, GLfloat g2, GLfloat b2)
{
    GLfloat x11;
    GLfloat y11;
    getAATrisVert(cx, cy, x1, y1, &x11, &y11, AA_VECT_LEN);
    GLfloat x22;
    GLfloat y22;
    getAATrisVert(cx, cy, x2, y2, &x22, &y22, AA_VECT_LEN);
    GLfloat rc = (r1 + r2)*0.5;
    GLfloat gc = (g1 + g2)*0.5;
    GLfloat bc = (b1 + b2)*0.5;

    glBegin(GL_TRIANGLES);
        glColor3f(r1, g1, b1);
        glVertex2f(x1, y1);
        glColor3f(r2, g2, b2);
        glVertex2f(x2, y2);
        glColor3f(rc, gc, bc);
        glVertex2f(cx, cy);

        glColor3f(r1, g1, b1);
        glVertex2f(x1, y1);
        glColor3f(r2, g2, b2);
        glVertex2f(x2, y2);
        glColor4f(r1, g1, b1, 0.f);
        glVertex2f(x11, y11);

        glColor4f(r1, g1, b1, 0.f);
        glVertex2f(x11, y11);
        glColor4f(r2, g2, b2, 0.f);
        glVertex2f(x22, y22);
        glColor3f(r2, g2, b2);
        glVertex2f(x2, y2);
    glEnd();
}

void ColorWidget::drawSolidCircle(GLfloat r, GLfloat cx, GLfloat cy, GLuint num_seg,
                                  GLfloat red, GLfloat green, GLfloat blue)
{
    float theta = 2 * PI / float(num_seg);
    float c = cosf(theta);//precalculate the sine and cosine
    float s = sinf(theta);
    float inner_x = r;
    float inner_y = 0;
    float last_inner_x = inner_x;
    float last_inner_y = inner_y;
    float inner_t;
    for(int ii = 0; ii < num_seg; ii++)
    {
        //apply the rotation matrix
        inner_t = inner_x;
        inner_x = c * inner_x - s * inner_y;
        inner_y = s * inner_t + c * inner_y;

        drawAACircTris(last_inner_x + cx, last_inner_y + cy,
                       inner_x + cx, inner_y + cy,
                       cx, cy,
                       red, green, blue,
                       red, green, blue);

        last_inner_x = inner_x;
        last_inner_y = inner_y;
    }
}

void ColorWidget::drawRect(GLfloat x, GLfloat y, GLfloat width, GLfloat height,
                           GLfloat r1, GLfloat g1, GLfloat b1, GLfloat a1,
                           GLfloat r2, GLfloat g2, GLfloat b2, GLfloat a2,
                           GLfloat r3, GLfloat g3, GLfloat b3, GLfloat a3,
                           GLfloat r4, GLfloat g4, GLfloat b4, GLfloat a4,
                           bool top_aa, bool bottom_aa, bool left_aa, bool right_aa)
{
    GLfloat x1 = x;
    GLfloat y1 = y;
    GLfloat x11 = x1 - AA_VECT_LEN;
    GLfloat y11 = y1 - AA_VECT_LEN;

    GLfloat x2 = x + width;
    GLfloat y2 = y;
    GLfloat x22 = x2 + AA_VECT_LEN;
    GLfloat y22 = y2 - AA_VECT_LEN;

    GLfloat x3 = x + width;
    GLfloat y3 = y + height;
    GLfloat x33 = x3 + AA_VECT_LEN;
    GLfloat y33 = y3 + AA_VECT_LEN;

    GLfloat x4 = x;
    GLfloat y4 = y + height;
    GLfloat x44 = x4 - AA_VECT_LEN;
    GLfloat y44 = y4 + AA_VECT_LEN;

    glBegin(GL_QUADS);
        glColor4f(r1, g1, b1, a1);
        glVertex2f(x1, y1);
        glColor4f(r2, g2, b2, a2);
        glVertex2f(x2, y2);
        glColor4f(r3, g3, b3, a3);
        glVertex2f(x3, y3);
        glColor4f(r4, g4, b4, a4);
        glVertex2f(x4, y4);

        if(top_aa)
        {
            glColor4f(r1, g1, b1, 0.f);
            glVertex2f(x11, y11);
            glColor4f(r2, g2, b2, 0.f);
            glVertex2f(x22, y22);
            glColor3f(r2, g2, b2);
            glVertex2f(x2, y2);
            glColor3f(r1, g1, b1);
            glVertex2f(x1, y1);
        }
        if(left_aa)
        {
            glColor4f(r1, g1, b1, 0.f);
            glVertex2f(x11, y11);
            glColor3f(r1, g1, b1);
            glVertex2f(x1, y1);
            glColor3f(r4, g4, b4);
            glVertex2f(x4, y4);
            glColor4f(r4, g4, b4, 0.f);
            glVertex2f(x44, y44);
        }
        if(right_aa)
        {
            glColor3f(r2, g2, b2);
            glVertex2f(x2, y2);
            glColor4f(r2, g2, b2, 0.f);
            glVertex2f(x22, y22);
            glColor4f(r3, g3, b3, 0.f);
            glVertex2f(x33, y33);
            glColor3f(r3, g3, b3);
            glVertex2f(x3, y3);
        }
        if(bottom_aa)
        {
            glColor3f(r4, g4, b4);
            glVertex2f(x4, y4);
            glColor3f(r3, g3, b3);
            glVertex2f(x3, y3);
            glColor4f(r3, g3, b3, 0.f);
            glVertex2f(x33, y33);
            glColor4f(r4, g4, b4, 0.f);
            glVertex2f(x44, y44);
        }
    glEnd();
}

void ColorWidget::drawRect(GLfloat x, GLfloat y, GLfloat width, GLfloat height,
                           GLfloat r1, GLfloat g1, GLfloat b1,
                           GLfloat r2, GLfloat g2, GLfloat b2,
                           GLfloat r3, GLfloat g3, GLfloat b3,
                           GLfloat r4, GLfloat g4, GLfloat b4,
                           bool top_aa, bool bottom_aa, bool left_aa, bool right_aa)
{
    drawRect(x, y, width, height,
             r1, g1, b1, 1.f,
             r2, g2, b2, 1.f,
             r3, g3, b3, 1.f,
             r4, g4, b4, 1.f,
             top_aa, bottom_aa, left_aa, right_aa);
}

