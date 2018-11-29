#ifndef GLWIDGET_H
#define GLWIDGET_H
#include <QOpenGLWidget>
#include "Colors/color.h"
#include "skiaincludes.h"

extern GLfloat AA_VECT_LEN;
extern GLfloat AA_SHARP_VECT_LEN;
class Gradient;

class GLWidget : public QOpenGLWidget {
    Q_OBJECT
public:
    GLWidget(QWidget *parent);
    ~GLWidget() {}

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
    static void drawAATris(const GLfloat &x1,
                           const GLfloat &y1,
                           const GLfloat &r1,
                           const GLfloat &g1,
                           const GLfloat &b1,
                           const GLfloat &x2,
                           const GLfloat &y2,
                           const GLfloat &r2,
                           const GLfloat &g2,
                           const GLfloat &b2,
                           const GLfloat &x3,
                           const GLfloat &y3,
                           const GLfloat &r3,
                           const GLfloat &g3,
                           const GLfloat &b3,
                           const GLboolean &e12_aa,
                           const GLboolean &e13_aa,
                           const GLboolean &e23_aa);
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
private:
    void initializeGL();
    bool mVisible = true;
protected:
    void resizeGL(int w, int h);
    SkColor4f mBgColor;
};

#endif // GLWIDGET_H
