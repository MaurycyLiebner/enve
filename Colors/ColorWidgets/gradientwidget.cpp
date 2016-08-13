#include "gradientwidget.h"
#include <QMouseEvent>
#include <QMenu>
#include <QAction>

GradientWidget::GradientWidget(QWidget *parent) : GLWidget(parent)
{
    setFixedHeight(6*20 + 10);
}

void GradientWidget::newGradient(Color color1, Color color2) {
    mGradients << new Gradient(color1, color2);
}

void GradientWidget::wheelEvent(QWheelEvent *event)
{
    if(event->y() > height()/2) return;
    if(event->delta() > 0) {
        if(mCenterGradientId + 1 >= mGradients.length()) return;
        mCenterGradientId = mCenterGradientId + 1;
    } else {
        if(mCenterGradientId - 1 < 0) return;
        mCenterGradientId = mCenterGradientId - 1;
    }
}

void GradientWidget::setCurrentColor(GLfloat h, GLfloat s, GLfloat v, GLfloat a)
{
    Color newColor;
    newColor.setHSV(h, s, v, a);
    mCurrentGradient->replaceColor(mCurrentColorId, newColor);
    repaint();
}

void GradientWidget::setCurrentGradient(Gradient *gradient)
{
    mCurrentColorId = 0;
    mCurrentGradient = gradient;
    repaint();
}

void GradientWidget::setCurrentGradient(int listId)
{
    if(listId >= mGradients.length()) return;
    setCurrentGradient(mGradients.at(listId));
}

void GradientWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->y() < height()/2 - 10) {
        int heightGrad = (height()/2 - 10)/3;
        setCurrentGradient(event->y()/heightGrad);
    } else if(event->y() > height()/2 && event->y() < 3*height()/4 && mCurrentGradient != NULL) {
        mCurrentColorId = event->x()*mCurrentGradient->colors.length()/width();
        repaint();
        if(event->button() == Qt::RightButton)
        {
            QMenu menu(this);
            menu.addAction("Delete Color");
            QAction *selected_action = menu.exec(event->globalPos());
            if(selected_action != NULL)
            {
                if(selected_action->text() == "Delete Color")
                {

                    if(mCurrentGradient->colors.count() < 2) {
                        mCurrentGradient->replaceColor(mCurrentColorId,
                                                       Color(0.f, 0.f, 0.f, 1.f));
                    } else {
                        mCurrentGradient->removeColor(mCurrentColorId);
                    }
                    mCurrentColorId = 0;
                    repaint();
                }
            }
            else
            {

            }
        }
    }
}

void GradientWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->y() > height()/2 && event->y() < 3*height()/4 && mCurrentGradient != NULL) {
            int colorId = event->x()*mCurrentGradient->colors.length()/width();
            if(colorId != mCurrentColorId) {
                mCurrentGradient->swapColors(mCurrentColorId, colorId);
                mCurrentColorId = colorId;
                repaint();
            }
    }
}

void GradientWidget::drawBorder(GLfloat xt, GLfloat yt, GLfloat wt, GLfloat ht) {
    drawRect(xt, yt, wt, 2.f,
             0.f, 0.f, 0.f, 1.f,
             0.f, 0.f, 0.f, 1.f,
             0.f, 0.f, 0.f, 1.f,
             0.f, 0.f, 0.f, 1.f,
             false, false, false, false);
    drawRect(xt, yt + ht - 2.f, wt, 2.f,
             0.f, 0.f, 0.f, 1.f,
             0.f, 0.f, 0.f, 1.f,
             0.f, 0.f, 0.f, 1.f,
             0.f, 0.f, 0.f, 1.f,
             false, false, false, false);
    drawRect(xt, yt + 2.f, 2.f, ht - 4.f,
             0.f, 0.f, 0.f, 1.f,
             0.f, 0.f, 0.f, 1.f,
             0.f, 0.f, 0.f, 1.f,
             0.f, 0.f, 0.f, 1.f,
             false, false, false, false);
    drawRect(xt + wt - 2.f, yt + 2.f, 2.f, ht - 4.f,
             0.f, 0.f, 0.f, 1.f,
             0.f, 0.f, 0.f, 1.f,
             0.f, 0.f, 0.f, 1.f,
             0.f, 0.f, 0.f, 1.f,
             false, false, false, false);

    xt += 2.f;
    yt += 2.f;
    wt -= 4.f;
    ht -= 4.f;
    drawRect(xt, yt, wt, 1.f,
             1.f, 1.f, 1.f, 1.f,
             1.f, 1.f, 1.f, 1.f,
             1.f, 1.f, 1.f, 1.f,
             1.f, 1.f, 1.f, 1.f,
             false, false, false, false);
    drawRect(xt, yt + ht - 1.f, wt, 1.f,
             1.f, 1.f, 1.f, 1.f,
             1.f, 1.f, 1.f, 1.f,
             1.f, 1.f, 1.f, 1.f,
             1.f, 1.f, 1.f, 1.f,
             false, false, false, false);
    drawRect(xt, yt + 1.f, 1.f, ht - 2.f,
             1.f, 1.f, 1.f, 1.f,
             1.f, 1.f, 1.f, 1.f,
             1.f, 1.f, 1.f, 1.f,
             1.f, 1.f, 1.f, 1.f,
             false, false, false, false);
    drawRect(xt + wt - 1.f, yt + 1.f, 1.f, ht - 2.f,
             1.f, 1.f, 1.f, 1.f,
             1.f, 1.f, 1.f, 1.f,
             1.f, 1.f, 1.f, 1.f,
             1.f, 1.f, 1.f, 1.f,
             false, false, false, false);
}

void GradientWidget::drawGradient(int id, GLfloat height, GLfloat cY, bool border) {
    Gradient *gradient = mGradients.at(id);

    int len = gradient->colors.length();
    Color nextColor = gradient->colors.first();
    GLfloat cX = 0.f;
    GLfloat segWidth = width()/(GLfloat)(len - 1);

    for(int i = 0; i < len - 1; i++) {
        Color color = nextColor;
        nextColor = gradient->colors.at(i + 1);

        drawRect(cX, cY, segWidth, height,
                 color.gl_r, color.gl_g, color.gl_b, color.gl_a,
                 nextColor.gl_r, nextColor.gl_g, nextColor.gl_b, nextColor.gl_a,
                 nextColor.gl_r, nextColor.gl_g, nextColor.gl_b, nextColor.gl_a,
                 color.gl_r, color.gl_g, color.gl_b, color.gl_a,
                 false, false, false, false);
        cX += segWidth;
    }
    if(gradient == mCurrentGradient && border) {
        drawBorder(0.f, cY, width(), height);
    }
}

void GradientWidget::paintGL()
{
    if(mCurrentGradient == NULL) return;
    GLfloat halfHeight = height()*0.5f;
    GLfloat quorterHeight = halfHeight*0.5f;
    GLfloat scrollItemHeight = (halfHeight - 10.f)*0.334f;

    GLfloat val1 = 0.5f;
    GLfloat val2 = 0.25f;
    for(int i = 0; i < width(); i += 7) {
        for(int j = 0; j < height(); j += 7) {
            GLfloat val = ((i + j) % 2 == 0) ? val1 : val2;
            drawSolidRect(i, j, 7, 7, val, val, val,
                          false, false, false, false);
        }
    }

    for(int i = (mCenterGradientId > 1) ? mCenterGradientId - 1 : 0;
        i < mCenterGradientId + 2 && i < mGradients.length();
        i++) {
        drawGradient(i, scrollItemHeight, i*scrollItemHeight);
    }
    drawSolidRect(0.f, halfHeight - 10.f, width(), 10.f, 1.f, 1.f, 1.f, 1.f,
                  false, false, false, false);

    int len = mCurrentGradient->colors.length();
    Color nextColor = mCurrentGradient->colors.first();
    GLfloat cX = 0.f;
    GLfloat segWidth = width()/(GLfloat)len;
    for(int i = 0; i < len; i++) {
        Color color = nextColor;
        drawRect(cX, halfHeight, segWidth, quorterHeight,
                 color.gl_r, color.gl_g, color.gl_b, color.gl_a,
                 color.gl_r, color.gl_g, color.gl_b, color.gl_a,
                 color.gl_r, color.gl_g, color.gl_b, color.gl_a,
                 color.gl_r, color.gl_g, color.gl_b, color.gl_a,
                 false, false, false, false);
        if(i == mCurrentColorId) {
            drawBorder(cX, halfHeight, segWidth, quorterHeight);
        }
        if(i + 1 == len) break;
        nextColor = mCurrentGradient->colors.at(i + 1);
        cX += segWidth;
    }
    drawGradient(mGradients.indexOf(mCurrentGradient), quorterHeight, halfHeight + quorterHeight, false);
}
