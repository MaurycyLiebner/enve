#include "glwidget.h"
#include "GUI/ColorWidgets/helpers.h"
#include "global.h"
#include "Animators/paintsettings.h"

GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent) {
//    setFormat(QSurfaceFormat::defaultFormat());
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    setMinimumSize(MIN_WIDGET_HEIGHT, MIN_WIDGET_HEIGHT);
    SkColor bgColor = SkColorSetARGB(255, 60, 60, 60);
    mBgColor = SkColor4f::FromColor(bgColor);
}

bool GLWidget::isVisible() {
    return mVisible;
}

bool GLWidget::isHidden() {
    return !mVisible;
}

void GLWidget::setVisible(bool b_t) {
    mVisible = b_t;
    QWidget::setVisible(b_t);
}

void GLWidget::show() {
    mVisible = true;
    QWidget::show();
}

void GLWidget::hide() {
    mVisible = false;
    QWidget::hide();
}

void GLWidget::initializeGL() {
    initializeOpenGLFunctions();

    glClearColor(mBgColor.fR, mBgColor.fG, mBgColor.fB, mBgColor.fA);
    assertNoGlErrors();
    //Set blending
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    assertNoGlErrors();

    iniPlainVShaderVAO(this, mPlainSquareVAO);
    assertNoGlErrors();
}
