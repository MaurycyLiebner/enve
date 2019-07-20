#ifndef GLWIDGET_H
#define GLWIDGET_H
#include <QOpenGLWidget>
#include "colorhelpers.h"
#include "skia/skiaincludes.h"
#include "glhelpers.h"
class Gradient;

class GLWidget : public QOpenGLWidget, protected QGL33 {
public:
    GLWidget(QWidget *parent);
    ~GLWidget() {
        if(mPlainSquareVAO) {
            makeCurrent();
            glDeleteBuffers(1, &mPlainSquareVAO);
            doneCurrent();
        }
    }
private:
    void initializeGL();
protected:
    GLuint mPlainSquareVAO = 0;
};

#endif // GLWIDGET_H
