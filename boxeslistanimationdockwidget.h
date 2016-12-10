#ifndef BOXESLISTANIMATIONDOCKWIDGET_H
#define BOXESLISTANIMATIONDOCKWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include "boxeslist.h"
#include "animatonwidgetscrollbar.h"
#include "keysview.h"
#include "boxeslistwidget.h"
#include <qscrollarea.h>

class AnimationDockWidget;

class BoxesListAnimationDockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BoxesListAnimationDockWidget(MainWindow *parent);
    BoxesListWidget *getBoxesList();
    KeysView *getKeysView();
    void setCurrentFrame(int frame);
    bool processUnfilteredKeyEvent(QKeyEvent *event);
    bool processFilteredKeyEvent(QKeyEvent *event);
    void previewFinished();
    void setPlaying(bool playing);
signals:
    void visibleRangeChanged(int, int);
private slots:
    void setCtrlsAlwaysVisible(bool ctrlsAlwaysVisible);

    void playPreview();

    void setGraphEnabled(bool recording);

    void setAllPointsRecord(bool allPointsRecord);
    void moveSlider(int val);
private:
    QScrollArea *mBoxesListScrollArea;

    MainWindow *mMainWindow;
    QVBoxLayout *mMainLayout;

    QWidget *mControlButtonsWidget;
    QHBoxLayout *mControlButtonsLayout;

    QPushButton *mGoToPreviousKeyButton;
    QPushButton *mGoToNextKeyButton;

    QPushButton *mPlayButton;

    QPushButton *mGraphEnabledButton;
    QPushButton *mAllPointsRecordButton;
    QPushButton *mCtrlsAlwaysVisible;

    AnimationDockWidget *mAnimationDockWidget;
    QHBoxLayout *mBoxesListKeysViewLayout;
    QVBoxLayout *mBoxesListLayout;
    QVBoxLayout *mKeysViewLayout;
    BoxesListWidget *mBoxesList;
    KeysView *mKeysView;
    AnimatonWidgetScrollBar *mFrameRangeScrollbar;
    AnimatonWidgetScrollBar *mAnimationWidgetScrollbar;
};

#endif // BOXESLISTANIMATIONDOCKWIDGET_H
