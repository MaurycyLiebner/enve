#ifndef GRADIENTWIDGET_H
#define GRADIENTWIDGET_H
#include "fillstrokesettings.h"
#include <QComboBox>
#include <QScrollArea>
#include "Gradients/gradientslistwidget.h"
#include "Gradients/currentgradientwidget.h"

class GradientWidget : public QWidget
{
    Q_OBJECT
public:
    GradientWidget(QWidget *parent, MainWindow *mainWindow);
    ~GradientWidget();

    void setCurrentGradient(Gradient *gradient,
                            const bool &emitChange = true);
    Gradient *getCurrentGradient();
    Color getCurrentColor();
    ColorAnimator *getCurrentColorAnimator();

    void moveColor(const int &x);

    void newGradient(const Color &color1 = Color(0.f, 0.f, 0.f, 1.f),
                     const Color &color2 = Color(0.f, 0.f, 0.f, 0.f));
    void newGradient(const int &fromGradientId);
    void removeGradient(int gradientId);

    void finishGradientTransform();
    void startGradientTransform();
    void saveGradientsToQuery(QSqlQuery *query);
    void clearAll();
    void saveGradientsToSqlIfPathSelected(QSqlQuery *query);
    void addGradientToList(Gradient *gradient);
    void removeGradientFromList(Gradient *toRemove);
    void startSelectedColorTransform();
    int getGradientIndex(Gradient *child);
    void updateNumberOfGradients();

    void drawGradients(const int &displayedTop,
                       const int &topGradientId,
                       const int &numberVisibleGradients,
                       const int &scrollItemHeight);

    void gradientLeftPressed(const int &gradId);
    void gradientContextMenuReq(const int &gradId,
                                const QPoint globalPos);
    void drawHoveredGradientBorder(const int &displayedTop,
                                   const int &topGradientId,
                                   const int &hoveredGradId,
                                   const int &scrollItemHeight);
    void drawCurrentGradientColors(const int &x, const int &y,
                                   const int &width, const int &height);
    void drawCurrentGradient(const int &x, const int &y,
                             const int &width, const int &height);
    void colorRightPress(const int &x, const QPoint &point);
    void colorLeftPress(const int &x);
    int getColorIdAtX(const int &x);
    void drawHoveredColorBorder(const int &hoveredX,
                                const int &colHeight);

    void updateAfterFrameChanged(const int &absFrame);
signals:
    void selectedColorChanged(ColorAnimator*);
    void currentGradientChanged(Gradient *gradient);
    void gradientSettingsChanged();
public slots:
    void resetColorIdIfEquals(Gradient *gradient, const int &id);
    void setCurrentColor(GLfloat h, GLfloat s, GLfloat v, GLfloat a = 1.f);
private slots:
    void updateAll();
private:
    QVBoxLayout *mMainLayout;
    GradientsListWidget *mGradientsListWidget;
    CurrentGradientWidget *mCurrentGradientWidget;

    int mNumberVisibleGradients = 6;
    int mHalfHeight = 64;
    int mQuorterHeight = 32;
    int mScrollItemHeight;

    MainWindow *mMainWindow;
    QList<QSharedPointer<Gradient> > mGradients;
    Gradient *mCurrentGradient = NULL;
    int mCurrentColorId = 0;
    int mCenterGradientId = 1;
    void setCurrentGradient(const int &listId);
    void setCurrentColorId(int id);
};

#endif // GRADIENTWIDGET_H
