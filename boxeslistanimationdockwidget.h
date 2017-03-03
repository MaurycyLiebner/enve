#ifndef BOXESLISTANIMATIONDOCKWIDGET_H
#define BOXESLISTANIMATIONDOCKWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include "animatonwidgetscrollbar.h"
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

class ChangeWidthWidget : public QWidget
{
    Q_OBJECT
public:
    ChangeWidthWidget(QWidget *boxesList, QWidget *parent = 0);

    void updatePos();

    void paintEvent(QPaintEvent *);

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);

    void enterEvent(QEvent *);

    void leaveEvent(QEvent *);

private:
    bool mHover = false;
    bool mPressed = false;
    int mPressX;
    QWidget *mBoxesList;
};

class AnimationDockWidget;
class BoxScrollWidget;

class BoxesListAnimationDockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BoxesListAnimationDockWidget(MainWindow *parent);
    BoxScrollWidget *getBoxesList();
    KeysView *getKeysView();
    bool processUnfilteredKeyEvent(QKeyEvent *event);
    bool processFilteredKeyEvent(QKeyEvent *event);
    void previewFinished();
    void setPlaying(bool playing);
    void updateSettingsForCurrentCanvas(Canvas *canvas);
public slots:
    void setRuleNone();
    void setRuleSelected();
    void setRuleAnimated();
    void setRuleNotAnimated();
    void setRuleVisible();
    void setRuleInvisible();
    void setRuleUnloced();
    void setRuleLocked();

    void setTargetAll();
    void setTargetCurrentCanvas();
    void setTargetCurrentGroup();

    void setSearchText(const QString &text);
    void setCurrentFrame(int frame);
    void setMinMaxFrame(const int &minFrame, const int &maxFrame);
signals:
    void visibleRangeChanged(int, int);
private slots:

    void setCtrlsAlwaysVisible(bool ctrlsAlwaysVisible);

    void playPreview();

    void setGraphEnabled(bool recording);

    void setAllPointsRecord(bool allPointsRecord);
    void moveSlider(int val);
private:
    QHBoxLayout *mBoxesListMenuLayout;
    QLineEdit *mSearchLine;
    QMenuBar *mBoxesListMenuBar;

    ScrollArea *mBoxesListScrollArea;

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
    QHBoxLayout *mBoxesListKeysViewLayout;
    QVBoxLayout *mBoxesListLayout;
    QVBoxLayout *mKeysViewLayout;
    BoxScrollWidget *mBoxesListWidget;
    KeysView *mKeysView;
    AnimatonWidgetScrollBar *mFrameRangeScrollbar;
    AnimatonWidgetScrollBar *mAnimationWidgetScrollbar;
};

#endif // BOXESLISTANIMATIONDOCKWIDGET_H
