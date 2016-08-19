#ifndef COLOR_H
#define COLOR_H
#include "GL/gl.h"
#include <QColor>
#include <QSqlQuery>
#include <QSqlRecord>

struct Color
{
    GLfloat gl_r = 1.0f;
    GLfloat gl_g = 1.0f;
    GLfloat gl_b = 1.0f;
    GLfloat gl_a = 1.0f;

    GLfloat gl_h = 0.0;
    GLfloat gl_s = 0.0;
    GLfloat gl_v = 1.0;

    QColor qcol = QColor(255, 255, 255);
    /*void setGLColorPremultipliedR(GLfloat r_t)
    {
        gl_r = r_t*gl_a;
        qcol.setRedF(r_t);
    }
    void setGLColorPremultipliedG(GLfloat g_t)
    {
        gl_g = g_t*gl_a;
        qcol.setGreenF(g_t);
    }
    void setGLColorPremultipliedB(GLfloat b_t)
    {
        gl_b = b_t*gl_a;
        qcol.setBlueF(b_t);
    }
    void setGLColorPremultipliedA(GLfloat a_t)
    {
        GLfloat da = a_t/gl_a;
        gl_r *= da;
        gl_g *= da;
        gl_b *= da;
        gl_a = a_t;
        qcol.setAlpha(a_t);
    }*/
    int saveToSql();

    void updateHSVFromRGB();
    void updateQColFromHSV();
    void setHSV(GLfloat h, GLfloat s, GLfloat v, GLfloat a = 1.f);
    void setRGB(GLfloat r_t, GLfloat g_t, GLfloat b_t);
    void updateRGBFromHSV();
    void setGLColor();
    void setGLClearColor();
    void setGLColorR(GLfloat r_t);
    void setGLColorG(GLfloat g_t);
    void setGLColorB(GLfloat b_t);
    void setQColor(QColor col_t);
    void setGLColorA(GLfloat a_t);
    Color(GLfloat r_t, GLfloat g_t, GLfloat b_t, GLfloat a_t = 1.0f);
    Color(int r_t, int g_t, int b_t, int a_t = 255);
    Color(int sqlId);
    Color();
};

#endif // COLOR_H
