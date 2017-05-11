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
    ~GradientWidget();

    void setCurrentGradient(Gradient *gradient);
    Gradient *getCurrentGradient();
    Color getCurrentColor();
    ColorAnimator *getCurrentColorAnimator();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void newGradient(Color color1 = Color(0.f, 0.f, 0.f, 1.f),
                     Color color2 = Color(0.f, 0.f, 0.f, 0.f));
    void newGradient(int fromGradientId);
    void removeGradient(int gradientId);

    void wheelEvent(QWheelEvent *event);

    void paintGL();

    void finishGradientTransform();
    void startGradientTransform();
    void saveGradientsToQuery(QSqlQuery *query);
    void clearAll();
    void saveGradientsToSqlIfPathSelected(QSqlQuery *query);
    void addGradientToList(Gradient *gradient);
    void removeGradientFromList(Gradient *toRemove);
    void startSelectedColorTransform();
    int getGradientIndex(Gradient *child);
signals:
    void selectedColorChanged(ColorAnimator*);
    void currentGradientChanged(Gradient *gradient);
    void gradientSettingsChanged();
public slots:
    void resetColorIdIfEquals(Gradient *gradient, const int &id);
    void setCurrentColor(GLfloat h, GLfloat s, GLfloat v, GLfloat a = 1.f);
private:
    MainWindow *mMainWindow;
    QList<QSharedPointer<Gradient> > mGradients;
    Gradient *mCurrentGradient = NULL;
    int mCurrentColorId = 0;
    int mCenterGradientId = 1;
    void setCurrentGradient(int listId);
    void setCurrentColorId(int id);
};

#endif // GRADIENTWIDGET_H
