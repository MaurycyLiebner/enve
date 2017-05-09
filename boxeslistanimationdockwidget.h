#ifndef BOXESLISTANIMATIONDOCKWIDGET_H
#define BOXESLISTANIMATIONDOCKWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include "animationwidgetscrollbar.h"
#include "keysview.h"
#include <qscrollarea.h>
#include <QScrollArea>
#include <QApplication>
#include <QScrollBar>
#include <QComboBox>
#include <QMenuBar>
#include <QLineEdit>
#include <QWidgetAction>
#include <QToolBar>
#include "BoxesList/OptimalScrollArea/scrollarea.h"
#include "boxeslistkeysviewwidget.h"
class VerticalWidgetsStack;

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
signals:
    void widthSet(int);
private:
    int mCurrentWidth = 400;
    bool mHover = false;
    bool mPressed = false;
    int mPressX;
};

class AnimationDockWidget;
class BoxScrollWidget;
#include "actionbutton.h"

class BoxesListAnimationDockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BoxesListAnimationDockWidget(MainWindow *parent);
    bool processUnfilteredKeyEvent(QKeyEvent *event);
    bool processFilteredKeyEvent(QKeyEvent *event);
    void previewFinished();
    void updateSettingsForCurrentCanvas(Canvas *canvas);
    void addNewBoxesListKeysViewWidgetBelow(BoxesListKeysViewWidget *widget);
    void clearAll();
public slots:
    void setCurrentFrame(int frame);
    void setMinMaxFrame(const int &minFrame, const int &maxFrame);
    void addNewBoxesListKeysViewWidget(int id = 0);
    void removeBoxesListKeysViewWidget(BoxesListKeysViewWidget *widget);
signals:
    void visibleRangeChanged(int, int);
private slots:
    void setResolutionFractionText(QString text);

    void setCtrlsAlwaysVisible(bool ctrlsAlwaysVisible);

    void playStopPreview(const bool &play);
    void playPreview();
    void stopPreview();

    void setLocalPivot(const bool &bT);

    void setTimelineMode();
    void setRenderMode();
private:
    QAction *mTimelineAction;
    QAction *mRenderAction;

    QToolBar *mToolBar;

    QMenuBar *mAddBoxesListKeysViewWidgetsBar = new QMenuBar(this);
    QList<BoxesListKeysViewWidget*> mBoxesListKeysViewWidgets;

    VerticalWidgetsStack *mBoxesListKeysViewStack;
    ChangeWidthWidget *mChww;
    MainWindow *mMainWindow;
    QVBoxLayout *mMainLayout;

    QLabel *mControlButtonsWidget;

    QComboBox *mResolutionComboBox;
//    QPushButton *mGoToPreviousKeyButton;
//    QPushButton *mGoToNextKeyButton;

    ActionButton *mPlayButton;

    ActionButton *mAllPointsRecordButton;
    ActionButton *mCtrlsAlwaysVisible;
    ActionButton *mLocalPivot;

    AnimationDockWidget *mAnimationDockWidget;

    AnimationWidgetScrollBar *mFrameRangeScrollbar;
    AnimationWidgetScrollBar *mAnimationWidgetScrollbar;
};

#endif // BOXESLISTANIMATIONDOCKWIDGET_H
