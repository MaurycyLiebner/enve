#ifndef H_WHEEL_SV_TRIANGLE_H
#define H_WHEEL_SV_TRIANGLE_H
#include <QOpenGLWidget>
#include "colorwidget.h"
#include <GL/gl.h>

enum VALUEFOCUS
{
    H,
    SV,
    NONE
};

class H_Wheel_SV_Triangle : public ColorWidget
{
    Q_OBJECT
public:
    H_Wheel_SV_Triangle(QWidget *parent_t = nullptr);
private:
    void paintGL();
    void resizeGL(int w, int h);

    void drawTriangle();
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *e);
    bool isInTriangle(QPoint pos_t);
    bool isInWheel(QPoint pos_t);
    void wheelInteraction(int x_t, int y_t);
    void triangleInteraction(int x_t, int y_t);

    void wheelEvent(QWheelEvent *e);
    void drawWheel();

    GLuint wheel_dim = 128;
    GLuint wheel_thickness = 20;
    GLfloat triangle_width = 64;
    GLuint triangle_tex_width = 64;
    GLuint triangle_tex_height = 64;

    int update_c = 0;
    VALUEFOCUS value_focus = NONE;
    float outer_circle_r = 0;
    float inner_circle_r = 0;
};

#endif // H_WHEEL_SV_TRIANGLE_H
