#ifndef ANIMATIONDOCKWIDGET_H
#define ANIMATIONDOCKWIDGET_H
#include <QToolBar>
#include <QVBoxLayout>
#include <QPushButton>
class ActionButton;
class KeysView;

class AnimationDockWidget : public QToolBar {
    Q_OBJECT
public:
    explicit AnimationDockWidget(QWidget *parent, KeysView *keysView);

protected:
    void paintEvent(QPaintEvent *);
signals:

public slots:

private:
    QHBoxLayout *mButtonsLayout;
    ActionButton *mActionLine;
    ActionButton *mActionCurve;
    ActionButton *mSmoothButton;
    ActionButton *mSymmetricButton;
    ActionButton *mCornerButton;
    ActionButton *mFitToHeightButton;
};

#endif // ANIMATIONDOCKWIDGET_H
