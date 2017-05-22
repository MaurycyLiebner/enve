#ifndef ANIMATIONDOCKWIDGET_H
#define ANIMATIONDOCKWIDGET_H
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include "actionbutton.h"

class MainWindow;
class KeysView;

class AnimationDockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AnimationDockWidget(QWidget *parent, KeysView *keysView);

protected:
    void paintEvent(QPaintEvent *);
signals:

public slots:

private:
    QHBoxLayout *mButtonsLayout;
    ActionButton *mSmoothButton;
    ActionButton *mSymmetricButton;
    ActionButton *mCornerButton;
    ActionButton *mFitToHeightButton;
};

#endif // ANIMATIONDOCKWIDGET_H
