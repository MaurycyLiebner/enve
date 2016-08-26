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
signals:

public slots:
private:
    QVBoxLayout *mMainLayout;

    QWidget *mControlButtonsWidget;
    QHBoxLayout *mControlButtonsLayout;
    QHBoxLayout *mControlsLayout;

    QPushButton *mGoToPreviousKeyButton;
    QPushButton *mGoToNextKeyButton;

    QPushButton *mPlayButton;

    QPushButton *mRecordButton;
    QPushButton *mAddKeyButton;
    QPushButton *mRemoveKeyButton;

    BoxesList *mBoxesList;
    AnimatonWidgetScrollBar *mFrameRangeScrollbar;
    AnimatonWidgetScrollBar *mAnimationWidgetScrollbar;
};

#endif // BOXESLISTANIMATIONDOCKWIDGET_H
