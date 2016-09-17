#ifndef BOXESLISTANIMATIONDOCKWIDGET_H
#define BOXESLISTANIMATIONDOCKWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include "boxeslist.h"
#include "animatonwidgetscrollbar.h"

class BoxesListAnimationDockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BoxesListAnimationDockWidget(MainWindow *parent);
    BoxesList *getBoxesList();
    void setCurrentFrame(int frame);
    bool processUnfilteredKeyEvent(QKeyEvent *event);
    bool processFilteredKeyEvent(QKeyEvent *event);
    void previewFinished();
    void setPlaying(bool playing);
signals:

public slots:

private slots:
    void playPreview();

    void setRecording(bool recording);

    void setAllPointsRecord(bool allPointsRecord);
private:
    MainWindow *mMainWindow;
    QVBoxLayout *mMainLayout;

    QWidget *mControlButtonsWidget;
    QHBoxLayout *mControlButtonsLayout;
    QHBoxLayout *mControlsLayout;

    QPushButton *mGoToPreviousKeyButton;
    QPushButton *mGoToNextKeyButton;

    QPushButton *mPlayButton;

    QPushButton *mRecordButton;
    QPushButton *mAllPointsRecordButton;
    QPushButton *mRemoveKeyButton;

    BoxesList *mBoxesList;
    AnimatonWidgetScrollBar *mFrameRangeScrollbar;
    AnimatonWidgetScrollBar *mAnimationWidgetScrollbar;
};

#endif // BOXESLISTANIMATIONDOCKWIDGET_H
