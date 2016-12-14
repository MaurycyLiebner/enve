#ifndef BOXESLIST_H
#define BOXESLIST_H

#include <QWidget>

class AnimationWidget;

class QrealAnimator;

class QrealPoint;

class QrealKey;

class Canvas;

class MainWindow;

class BoxesListWidget;

#include <QPainter>
class ChangeWidthWidget : public QWidget
{
    Q_OBJECT
public:
    ChangeWidthWidget(QWidget *boxesList, QWidget *parent = 0);

    void updatePos();

    void paintEvent(QPaintEvent *) {
        QPainter p(this);
        if(mouseGrabber() == this) {
            p.fillRect(rect(), Qt::black);
        }
        p.end();
    }

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
private:
    int mPressX;
    QWidget *mBoxesList;
};

#endif // BOXESLIST_H
