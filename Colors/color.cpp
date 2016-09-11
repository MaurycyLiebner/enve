#include "color.h"
#include "helpers.h"
#include <QDebug>


int Color::saveToSql()
{
    QSqlQuery query;
    query.exec(QString("INSERT INTO color (hue, saturation, value, alpha) "
                "VALUES (%1, %2, %3, %4)").
                arg(gl_h, 0, 'f').
                arg(gl_s, 0, 'f').
                arg(gl_v, 0, 'f').
                arg(gl_a, 0, 'f') );
    return query.lastInsertId().toInt();
}

void Color::updateHSVFromRGB()
{
    gl_h = gl_r;
    gl_s = gl_g;
    gl_v = gl_b;
    rgb_to_hsv_float(&gl_h, &gl_s, &gl_v);
}
void Color::updateQColFromHSV()
{
    qcol.setHsvF(gl_h, gl_s, gl_v, gl_a);
}
void Color::setHSV(GLfloat h, GLfloat s, GLfloat v, GLfloat a)
{
    gl_h = h;
    gl_s = s;
    gl_v = v;
    gl_a = a;
    updateQColFromHSV();
    updateRGBFromHSV();
}

void Color::setHSL(GLfloat h, GLfloat s, GLfloat l, GLfloat a)
{
    hsl_to_hsv(&h, &s, &l);
    setHSV(h, s, l, a);
}


void Color::setRGB(GLfloat r_t, GLfloat g_t, GLfloat b_t, GLfloat a)
{
    gl_r = r_t;
    gl_g = g_t;
    gl_b = b_t;
    gl_a = a;
    qcol.setRgbF(r_t, g_t, b_t);
    updateHSVFromRGB();
}
void Color::updateRGBFromHSV()
{
    gl_r = gl_h;
    gl_g = gl_s;
    gl_b = gl_v;
    hsv_to_rgb_float(&gl_r, &gl_g, &gl_b);
}

void Color::setGLColor()
{
    glColor4f(gl_r, gl_g, gl_b, gl_a);
}
void Color::setGLClearColor()
{
    glClearColor(gl_r, gl_g, gl_b, gl_a);
}
void Color::setGLColorR(GLfloat r_t)
{
    gl_r = r_t;
    qcol.setRedF(r_t);
    updateHSVFromRGB();
}
void Color::setGLColorG(GLfloat g_t)
{
    gl_g = g_t;
    qcol.setGreenF(g_t);
    updateHSVFromRGB();
}
void Color::setGLColorB(GLfloat b_t)
{
    gl_b = b_t;
    qcol.setBlueF(b_t);
    updateHSVFromRGB();
}
void Color::setQColor(QColor col_t)
{
    gl_r = col_t.redF();
    gl_g = col_t.greenF();
    gl_b = col_t.blueF();
    qcol = col_t;
    updateHSVFromRGB();
}
void Color::setGLColorA(GLfloat a_t)
{
    gl_a = a_t;
    qcol.setAlpha(a_t*255);
}

Color::Color(GLfloat r_t, GLfloat g_t, GLfloat b_t, GLfloat a_t)
{
    gl_r = r_t;
    gl_g = g_t;
    gl_b = b_t;
    gl_a = a_t;
    qcol.setRgbF(r_t, g_t, b_t, a_t);
    updateHSVFromRGB();
}
Color::Color(int r_t, int g_t, int b_t, int a_t)
{
    qcol.setRgb(r_t, g_t, b_t, a_t);
    gl_r = r_t/255.0f;
    gl_g = g_t/255.0f;
    gl_b = b_t/255.0f;
    gl_a = a_t/255.0f;
    updateHSVFromRGB();
}

Color::Color(int sqlId)
{
    QSqlQuery query;
    QString queryStr = QString("SELECT * FROM color WHERE id = %1").
            arg(sqlId);
    if(query.exec(queryStr) ) {
        query.next();
        int idHue = query.record().indexOf("hue");
        GLfloat h = query.value(idHue).toReal();
        int idSaturation = query.record().indexOf("saturation");
        GLfloat s = query.value(idSaturation).toReal();
        int idValue = query.record().indexOf("value");
        GLfloat v = query.value(idValue).toReal();
        int idAlpha = query.record().indexOf("alpha");
        GLfloat a = query.value(idAlpha).toReal();
        setHSV(h, s, v, a);
    } else {
        qDebug() << "Could not load color with id " << sqlId;
    }
}

Color::Color()
{
    Color(0, 0, 0, 255);
}
