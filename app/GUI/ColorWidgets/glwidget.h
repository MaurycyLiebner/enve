#ifndef GLWIDGET_H
#define GLWIDGET_H
#include <QOpenGLWidget>
#include "colorhelpers.h"
#include "skia/skiaincludes.h"
#include "glhelpers.h"
class Gradient;

class GLWidget : public QOpenGLWidget, protected QGL33c {
public:
    GLWidget(QWidget *parent);
    ~GLWidget() {}

    bool isVisible();
    bool isHidden();
    void setVisible(bool b_t);
    void show();
    void hide();
private:
    void initializeGL();
    bool mVisible = true;
protected:
    GLuint mPlainSquareVAO;
    SkColor4f mBgColor;
};

#endif // GLWIDGET_H
