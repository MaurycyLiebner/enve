#include "singlegradientwidget.h"
#include "fillstrokesettings.h"

SingleGradientWidget::SingleGradientWidget(QWidget *parent) :
    GLWidget(parent) {

}

void SingleGradientWidget::paintGL() {
    drawMeshBg();
    if(!mTargetGradient->isEmpty()) {
        int len = mTargetGradient->getColorCount();
        Color nextColor = mTargetGradient->getCurrentColorAt(0);
        GLfloat cX = 0.f;
        GLfloat segWidth = width()/(GLfloat)(len - 1);

        for(int i = 0; i < len - 1; i++) {
            Color color = nextColor;
            nextColor = mTargetGradient->getCurrentColorAt(i + 1);

            drawRect(cX, 0.f, segWidth, height(),
                     color.gl_r, color.gl_g, color.gl_b, color.gl_a,
                     nextColor.gl_r, nextColor.gl_g, nextColor.gl_b, nextColor.gl_a,
                     nextColor.gl_r, nextColor.gl_g, nextColor.gl_b, nextColor.gl_a,
                     color.gl_r, color.gl_g, color.gl_b, color.gl_a,
                     false, false, false, false);
            cX += segWidth;
        }
    }
    if(mSelected) {
        drawBorder(0.f, 0.f, width(), height());
    }
}

void SingleGradientWidget::setSelected(const bool &bT) {
    mSelected = bT;
}
