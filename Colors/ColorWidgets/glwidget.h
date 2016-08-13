#ifndef GLWIDGET_H
#define GLWIDGET_H
#include <QOpenGLWidget>
#include "Colors/color.h"

extern GLfloat AA_VECT_LEN;
extern GLfloat AA_SHARP_VECT_LEN;

class GLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    GLWidget(QWidget *parent);
    bool isVisible();
    bool isHidden();
    void setVisible(bool b_t);
    void show();
    void hide();

protected:
    Color bgColor;

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
    void drawRect(GLfloat x, GLfloat y, GLfloat width, GLfloat height,
                  GLfloat r1, GLfloat g1, GLfloat b1, GLfloat a1,
                  GLfloat r2, GLfloat g2, GLfloat b2, GLfloat a2,
                  GLfloat r3, GLfloat g3, GLfloat b3, GLfloat a3,
                  GLfloat r4, GLfloat g4, GLfloat b4, GLfloat a4,
                  bool top_aa, bool bottom_aa, bool left_aa, bool right_aa);
    void drawSolidRect(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLfloat r, GLfloat g, GLfloat b, GLfloat a, bool top_aa, bool bottom_aa, bool left_aa, bool right_aa);
private:
    bool visible = true;
    void initializeGL();
};

#endif // GLWIDGET_H
