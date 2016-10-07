#ifndef ANIMATIONDOCKWIDGET_H
#define ANIMATIONDOCKWIDGET_H
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>

class MainWindow;
class BoxesList;

class AnimationDockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AnimationDockWidget(QWidget *parent, BoxesList *boxesList);

protected:
    void paintEvent(QPaintEvent *);
signals:

public slots:

private:
    QHBoxLayout *mButtonsLayout;
    QPushButton *mSmoothButton;
    QPushButton *mSymmetricButton;
    QPushButton *mCornerButton;
    QPushButton *mTwoSideCtrlButton;
    QPushButton *mRightSideCtrlButton;
    QPushButton *mLeftSideCtrlButton;
    QPushButton *mNoSideCtrlButton;
};

#endif // ANIMATIONDOCKWIDGET_H
