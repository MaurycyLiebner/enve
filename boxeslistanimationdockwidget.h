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
#include "BoxesList/OptimalScrollArea/scrollarea.h"
#include "boxeslistkeysviewwidget.h"

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

class BoxesListAnimationDockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BoxesListAnimationDockWidget(MainWindow *parent);
    bool processUnfilteredKeyEvent(QKeyEvent *event);
    bool processFilteredKeyEvent(QKeyEvent *event);
    void previewFinished();
    void setPlaying(bool playing);
    void updateSettingsForCurrentCanvas(Canvas *canvas);
    void addNewBoxesListKeysViewWidget(const int &id);
public slots:
    void setCurrentFrame(int frame);
    void setMinMaxFrame(const int &minFrame, const int &maxFrame);
signals:
    void visibleRangeChanged(int, int);
private slots:

    void setCtrlsAlwaysVisible(bool ctrlsAlwaysVisible);

    void playPreview();

    void setGraphEnabled(bool recording);

    void setAllPointsRecord(bool allPointsRecord);
private:
    QList<BoxesListKeysViewWidget*> mBoxesListKeysViewWidgets;

    ChangeWidthWidget *mChww;
    MainWindow *mMainWindow;
    QVBoxLayout *mMainLayout;

    QLabel *mControlButtonsWidget;
    QHBoxLayout *mControlButtonsLayout;

    QComboBox *mResolutionComboBox;
//    QPushButton *mGoToPreviousKeyButton;
//    QPushButton *mGoToNextKeyButton;

    QPushButton *mPlayButton;

    QPushButton *mGraphEnabledButton;
    QPushButton *mAllPointsRecordButton;
    QPushButton *mCtrlsAlwaysVisible;

    AnimationDockWidget *mAnimationDockWidget;

    AnimationWidgetScrollBar *mFrameRangeScrollbar;
    AnimationWidgetScrollBar *mAnimationWidgetScrollbar;
};

#endif // BOXESLISTANIMATIONDOCKWIDGET_H
