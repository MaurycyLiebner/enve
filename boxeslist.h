#ifndef BOXESLIST_H
#define BOXESLIST_H

#include <QWidget>

const qreal LIST_ITEM_HEIGHT = 20;
const qreal LIST_ITEM_MAX_WIDTH = 200;
const qreal LIST_ITEM_CHILD_INDENT = 20;

class Canvas;

class MainWindow;

class BoxesList : public QWidget
{
    Q_OBJECT
public:
    explicit BoxesList(Canvas *canvas, MainWindow *parent = 0);
    void scheduleRepaint();
    void repaintIfNeeded();

    static QPixmap *VISIBLE_PIXMAP;
    static QPixmap *INVISIBLE_PIXMAP;
    static QPixmap *HIDE_CHILDREN;
    static QPixmap *SHOW_CHILDREN;
    static QPixmap *LOCKED_PIXMAP;
    static QPixmap *UNLOCKED_PIXMAP;
protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *e);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
signals:

public slots:

private:
    bool mRepaintScheduled = false;
    Canvas *mCanvas;
    QRectF mViewedRect;
    MainWindow *mMainWindow;
};

#endif // BOXESLIST_H
