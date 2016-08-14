#ifndef GRADIENTWIDGET_H
#define GRADIENTWIDGET_H
#include "glwidget.h"
#include "fillstrokesettings.h"
#include <QComboBox>

class GradientWidget : public GLWidget
{
    Q_OBJECT
public:
    GradientWidget(QWidget *parent, MainWindow *mainWindow);

    void setCurrentGradient(Gradient *gradient);
    Gradient *getCurrentGradient();
    Color getCurrentColor();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void newGradient(Color color1 = Color(0.f, 0.f, 0.f, 1.f),
                     Color color2 = Color(0.f, 0.f, 0.f, 0.f));

    void wheelEvent(QWheelEvent *event);

    void paintGL();

    void finishGradientTransform();
    void startGradientTransform();
signals:
    void selectedColorChanged(GLfloat h, GLfloat s, GLfloat v, GLfloat a = 1.f);
    void currentGradientChanged(Gradient *gradient);
    void gradientSettingsChanged();
public slots:
    void setCurrentColor(GLfloat h, GLfloat s, GLfloat v, GLfloat a = 1.f);
private:
    MainWindow *mMainWindow;
    QList<Gradient*> mGradients;
    Gradient *mCurrentGradient = NULL;
    int mCurrentColorId = 0;
    int mCenterGradientId = 1;
    void drawBorder(GLfloat xt, GLfloat yt, GLfloat wt, GLfloat ht);
    void setCurrentGradient(int listId);
    void setCurrentColorId(int id);
    void drawGradient(int id, GLfloat height, GLfloat cY, bool border = true);
};

#endif // GRADIENTWIDGET_H
