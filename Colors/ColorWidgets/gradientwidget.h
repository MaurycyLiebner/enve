#ifndef GRADIENTWIDGET_H
#define GRADIENTWIDGET_H
#include "fillstrokesettings.h"
#include <QComboBox>
#include <QScrollArea>
#include "Gradients/gradientslistwidget.h"
#include "Gradients/currentgradientwidget.h"

class GradientWidget : public QWidget {
    Q_OBJECT
public:
    GradientWidget(QWidget *parent, MainWindow *mainWindow);
    ~GradientWidget();

    void setCurrentGradient(Gradient *gradient,
                            const bool &emitChange = true);
    Gradient *getCurrentGradient();
    QColor getCurrentColor();
    ColorAnimator *getCurrentColorAnimator();

    void moveColor(const int &x);

    void newGradient(const QColor &color1 = QColor(0, 0, 0),
                     const QColor &color2 = QColor(0, 0, 0));
    void newGradient(const int &fromGradientId);
    void removeGradient(const int &gradientId);

    void finishGradientTransform();
    void startGradientTransform();
    void clearAll();
    void addGradientToList(const GradientQSPtr &gradient);
    void removeGradientFromList(const GradientQSPtr& toRemove);
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

    void clearGradientsLoadIds();
    void setGradientLoadIds();
    void writeGradients(QIODevice *target);
    void readGradients(QIODevice *target);
signals:
    void selectedColorChanged(ColorAnimator*);
    void currentGradientChanged(Gradient *gradient);
    void gradientSettingsChanged();
public slots:
    void resetColorIdIfEquals(Gradient *gradient, const int &id);
    void setCurrentColor(const QColor &col);
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
    Gradient *mCurrentGradient = nullptr;
    int mCurrentColorId = 0;
    int mCenterGradientId = 1;
    void setCurrentGradient(const int &listId);
    void setCurrentColorId(int id);
};

#endif // GRADIENTWIDGET_H
