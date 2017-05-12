#ifndef GLWIDGET_H
#define GLWIDGET_H
#include <QOpenGLWidget>
#include "Colors/color.h"

extern GLfloat AA_VECT_LEN;
extern GLfloat AA_SHARP_VECT_LEN;
class Gradient;

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

    static void drawBorder(GLfloat xt,
                           GLfloat yt,
                           GLfloat wt,
                           GLfloat ht);
    static void normalize(GLfloat *x_t,
                          GLfloat *y_t,
                          GLfloat dest_len);
    static void getAATrisVert(GLfloat center_x,
                              GLfloat center_y,
                              GLfloat x_t,
                              GLfloat y_t,
                              GLfloat *result_x,
                              GLfloat *result_y,
                              GLfloat blurriness);
    static void drawSolidAATris(GLfloat x1,
                                GLfloat y1,
                                GLfloat x2,
                                GLfloat y2,
                                GLfloat x3,
                                GLfloat y3,
                                GLfloat r,
                                GLfloat g,
                                GLfloat b,
                                GLboolean e12_aa,
                                GLboolean e13_aa,
                                GLboolean e23_aa);
    static void drawAATris(GLfloat x1,
                           GLfloat y1,
                           GLfloat r1,
                           GLfloat g1,
                           GLfloat b1,
                           GLfloat x2,
                           GLfloat y2,
                           GLfloat r2,
                           GLfloat g2,
                           GLfloat b2,
                           GLfloat x3,
                           GLfloat y3,
                           GLfloat r3,
                           GLfloat g3,
                           GLfloat b3,
                           GLboolean e12_aa,
                           GLboolean e13_aa,
                           GLboolean e23_aa);
    static void drawAACircTris(GLfloat x1,
                               GLfloat y1,
                               GLfloat x2,
                               GLfloat y2,
                               GLfloat cx,
                               GLfloat cy,
                               GLfloat r1,
                               GLfloat g1,
                               GLfloat b1,
                               GLfloat r2,
                               GLfloat g2,
                               GLfloat b2);
    static void drawSubTris(GLfloat x1,
                            GLfloat y1,
                            GLfloat h1,
                            GLfloat s1,
                            GLfloat v1,
                            GLfloat x2,
                            GLfloat y2,
                            GLfloat h2,
                            GLfloat s2,
                            GLfloat v2,
                            GLfloat x3,
                            GLfloat y3,
                            GLfloat h3,
                            GLfloat s3,
                            GLfloat v3,
                            int n_next,
                            GLboolean e12_aa,
                            GLboolean e13_aa,
                            GLboolean e23_aa);
    static void getAAVect(GLfloat center_x,
                          GLfloat center_y,
                          GLfloat x_t,
                          GLfloat y_t,
                          GLfloat *result_x,
                          GLfloat *result_y,
                          GLfloat blurrines);
    static void drawSolidCircle(GLfloat r,
                                GLfloat cx,
                                GLfloat cy,
                                GLuint num_seg,
                                GLfloat red,
                                GLfloat green,
                                GLfloat blue);
    static void drawRect(GLfloat x,
                         GLfloat y,
                         GLfloat width,
                         GLfloat height,
                         GLfloat r1,
                         GLfloat g1,
                         GLfloat b1,
                         GLfloat r2,
                         GLfloat g2,
                         GLfloat b2,
                         GLfloat r3,
                         GLfloat g3,
                         GLfloat b3,
                         GLfloat r4,
                         GLfloat g4,
                         GLfloat b4,
                         GLboolean top_aa = true,
                         GLboolean bottom_aa = true,
                         GLboolean left_aa = true,
                         GLboolean right_aa = true);
    static void drawSolidRect(GLfloat x,
                              GLfloat y,
                              GLfloat width,
                              GLfloat height,
                              GLfloat r,
                              GLfloat g,
                              GLfloat b,
                              GLboolean top_aa = true,
                              GLboolean bottom_aa = true,
                              GLboolean left_aa = true,
                              GLboolean right_aa = true);
    static void drawSolidRectCenter(GLfloat cx,
                                    GLfloat cy,
                                    GLfloat width,
                                    GLfloat height,
                                    GLfloat r,
                                    GLfloat g,
                                    GLfloat b,
                                    GLboolean top_aa = true,
                                    GLboolean bottom_aa = true,
                                    GLboolean left_aa = true,
                                    GLboolean right_aa = true);
    static void drawRect(GLfloat x,
                         GLfloat y,
                         GLfloat width,
                         GLfloat height,
                         GLfloat r1,
                         GLfloat g1,
                         GLfloat b1,
                         GLfloat a1,
                         GLfloat r2,
                         GLfloat g2,
                         GLfloat b2,
                         GLfloat a2,
                         GLfloat r3,
                         GLfloat g3,
                         GLfloat b3,
                         GLfloat a3,
                         GLfloat r4,
                         GLfloat g4,
                         GLfloat b4,
                         GLfloat a4,
                         GLboolean top_aa,
                         GLboolean bottom_aa,
                         GLboolean left_aa,
                         GLboolean right_aa);
    static void drawSolidRect(GLfloat x,
                              GLfloat y,
                              GLfloat width,
                              GLfloat height,
                              GLfloat r,
                              GLfloat g,
                              GLfloat b,
                              GLfloat a,
                              GLboolean top_aa,
                              GLboolean bottom_aa,
                              GLboolean left_aa,
                              GLboolean right_aa);
    static void drawMeshBg(const int &x,
                           const int &y,
                           const int &width,
                           const int &height);
    void drawMeshBg();
    static void drawGradient(Gradient *gradient,
                             const int &x,
                             const int &y,
                             const int &width,
                             const int &height);
    static void drawHoverBorder(GLfloat xt,
                                GLfloat yt,
                                GLfloat wt,
                                GLfloat ht);
protected:
    Color bgColor;

    void resizeGL(int w, int h);
private:
    bool visible = true;
    void initializeGL();
};

#endif // GLWIDGET_H
