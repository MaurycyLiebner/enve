#ifndef ANIMATIONDOCKWIDGET_H
#define ANIMATIONDOCKWIDGET_H
#include <QWidget>
#include <QVBoxLayout>
#include "animationwidget.h"
#include "animatonwidgetscrollbar.h"

class AnimationDockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AnimationDockWidget(QWidget *parent = 0);

signals:

public slots:
private:
    QVBoxLayout *mMainLayout;
    AnimatonWidgetScrollBar *mScrollBar;
    AnimationWidget *mAnimationWidget;
};

#endif // ANIMATIONDOCKWIDGET_H
