#ifndef BOXESLIST_H
#define BOXESLIST_H

#include <QWidget>

class AnimationWidget;

const qreal LIST_ITEM_HEIGHT = 20;
const qreal LIST_ITEM_MAX_WIDTH = 250;
const qreal LIST_ITEM_CHILD_INDENT = 20;

class QrealAnimator;

class QrealPoint;

class QrealKey;

class Canvas;

class MainWindow;

class BoxesList : public QWidget
{
    Q_OBJECT
public:
    explicit BoxesList(QWidget *parent = 0);

    static QPixmap *VISIBLE_PIXMAP;
    static QPixmap *INVISIBLE_PIXMAP;
    static QPixmap *HIDE_CHILDREN;
    static QPixmap *SHOW_CHILDREN;
    static QPixmap *LOCKED_PIXMAP;
    static QPixmap *UNLOCKED_PIXMAP;
    static QPixmap *ANIMATOR_CHILDREN_VISIBLE;
    static QPixmap *ANIMATOR_CHILDREN_HIDDEN;
    static QPixmap *ANIMATOR_RECORDING;
    static QPixmap *ANIMATOR_NOT_RECORDING;

    bool processFilteredKeyEvent(QKeyEvent *event);
    void translateViewed(qreal translateBy);
    qreal getViewedTop();
    qreal getViewedBottom();

    void handleWheelEvent(QWheelEvent *event);
protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *e);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    Canvas *mCanvas;
    qreal mViewedTop = 0.;
    qreal mViewedBottom;
    MainWindow *mMainWindow;
};

#endif // BOXESLIST_H
