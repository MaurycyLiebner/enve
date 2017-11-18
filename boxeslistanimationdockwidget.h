#ifndef BOXESLISTANIMATIONDOCKWIDGET_H
#define BOXESLISTANIMATIONDOCKWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <qscrollarea.h>
#include <QScrollArea>
#include <QApplication>
#include <QScrollBar>
#include <QComboBox>
#include <QMenuBar>
#include <QLineEdit>
#include <QWidgetAction>
#include <QToolBar>
class AnimationWidgetScrollBar;
class BoxesListKeysViewWidget;
class VerticalWidgetsStack;
class RenderWidget;
class MainWindow;
class AnimationDockWidget;
class RenderWidget;
class ActionButton;
class Canvas;

class ChangeWidthWidget : public QWidget
{
    Q_OBJECT
public:
    ChangeWidthWidget(QWidget *parent = 0);

    void updatePos();

    void paintEvent(QPaintEvent *);

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);

    void enterEvent(QEvent *);

    void leaveEvent(QEvent *);
    int getCurrentWidth() {
        return mCurrentWidth;
    }

signals:
    void widthSet(int);
private:
    int mCurrentWidth = 400;
    bool mHover = false;
    bool mPressed = false;
    int mPressX;
};

class BoxesListAnimationDockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BoxesListAnimationDockWidget(MainWindow *parent);
    bool processKeyEvent(QKeyEvent *event);
    void previewFinished();
    void previewBeingPlayed();
    void previewBeingRendered();
    void previewPaused();

    void updateSettingsForCurrentCanvas(Canvas *canvas);
    void addNewBoxesListKeysViewWidgetBelow(BoxesListKeysViewWidget *widget);
    void clearAll();

    RenderWidget *getRenderWidget();
public slots:
    void setCurrentFrame(const int &frame);
    void setMinMaxFrame(const int &minFrame,
                        const int &maxFrame);
    void addNewBoxesListKeysViewWidget(int id = 0);
    void removeBoxesListKeysViewWidget(
            BoxesListKeysViewWidget *widget);
signals:
    void visibleRangeChanged(int, int);
private slots:
    void setResolutionFractionText(QString text);

    void interruptPreview();

    void setLocalPivot(const bool &bT);
    void setBonesSelectionEnabled(const bool &bT);

    void setTimelineMode();
    void setRenderMode();
    void playPreview();
    void renderPreview();
    void pausePreview();
    void resumePreview();
private:
    QAction *mTimelineAction;
    QAction *mRenderAction;

    QToolBar *mToolBar;
    QList<BoxesListKeysViewWidget*> mBoxesListKeysViewWidgets;

    VerticalWidgetsStack *mBoxesListKeysViewStack;
    ChangeWidthWidget *mChww;
    MainWindow *mMainWindow;

    RenderWidget *mRenderWidget;
    QWidget *mTimelineWidget;
    QVBoxLayout *mTimelineLayout;

    QVBoxLayout *mMainLayout;

    QLabel *mControlButtonsWidget;

    QComboBox *mResolutionComboBox;
//    QPushButton *mGoToPreviousKeyButton;
//    QPushButton *mGoToNextKeyButton;

    ActionButton *mPlayButton;
    ActionButton *mStopButton;

    ActionButton *mLocalPivot;
    ActionButton *mBonesSelection;

    AnimationDockWidget *mAnimationDockWidget;

    AnimationWidgetScrollBar *mFrameRangeScrollbar;
    AnimationWidgetScrollBar *mAnimationWidgetScrollbar;
};

#endif // BOXESLISTANIMATIONDOCKWIDGET_H
