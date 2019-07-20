#include "glwidget.h"
#include "colorhelpers.h"
#include "global.h"
#include "Animators/paintsettingsanimator.h"

GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent) {
//    setFormat(QSurfaceFormat::defaultFormat());
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    setMinimumSize(MIN_WIDGET_DIM, MIN_WIDGET_DIM);
}

void GLWidget::initializeGL() {
    if(!initializeOpenGLFunctions())
        RuntimeThrow("Initializing GL functions failed.");

    glClearColor(0.235f, 0.235f, 0.235f, 1.f);
    //Set blending
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    iniPlainVShaderVAO(this, mPlainSquareVAO);
}
