#ifndef GRADIENTWIDGET_H
#define GRADIENTWIDGET_H
#include "glwidget.h"
#include "fillstrokesettings.h"
#include <QComboBox>

class GradientWidget : public GLWidget
{
    Q_OBJECT
public:
    GradientWidget(QWidget *parent);

    void setCurrentColor(GLfloat h, GLfloat s, GLfloat v, GLfloat a = 1.f);
    void setCurrentGradient(Gradient *gradient);
    void setCurrentGradient(int listId);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void newGradient(Color color1 = Color(0.f, 0.f, 0.f, 1.f),
                     Color color2 = Color(0.f, 0.f, 0.f, 0.f));
    void wheelEvent(QWheelEvent *event);
    void drawGradient(int id, GLfloat height, GLfloat cY, bool border = true);
    void paintGL();
private:
    QList<Gradient*> mGradients;
    Gradient *mCurrentGradient = NULL;
    int mCurrentColorId = 0;
    int mCenterGradientId = 1;
    void drawBorder(GLfloat xt, GLfloat yt, GLfloat wt, GLfloat ht);
};

#endif // GRADIENTWIDGET_H
