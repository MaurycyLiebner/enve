#ifndef ANIMATIONDOCKWIDGET_H
#define ANIMATIONDOCKWIDGET_H
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include "animationwidget.h"
#include "animatonwidgetscrollbar.h"

class AnimationDockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AnimationDockWidget(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *);
signals:

public slots:

private:
    QVBoxLayout *mMainLayout;
    QHBoxLayout *mButtonsLayout;
    QPushButton *mSmoothButton;
    QPushButton *mSymmetricButton;
    QPushButton *mCornerButton;
    QPushButton *mTwoSideCtrlButton;
    QPushButton *mRightSideCtrlButton;
    QPushButton *mLeftSideCtrlButton;
    QPushButton *mNoSideCtrlButton;
    AnimatonWidgetScrollBar *mScrollBar;
    AnimationWidget *mAnimationWidget;
};

#endif // ANIMATIONDOCKWIDGET_H
