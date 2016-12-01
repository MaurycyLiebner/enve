#ifndef BOXESLIST_H
#define BOXESLIST_H

#include <QWidget>

class AnimationWidget;

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

    static qreal getListItemHeight() { return LIST_ITEM_HEIGHT; }
    static qreal getListItemMaxWidth() { return LIST_ITEM_MAX_WIDTH; }
    static qreal getListItemChildIndent() { return LIST_ITEM_CHILD_INDENT; }
    static void setListItemMaxWidth(int widthT) { LIST_ITEM_MAX_WIDTH = widthT; }
protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *e);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    static qreal LIST_ITEM_HEIGHT;
    static qreal LIST_ITEM_MAX_WIDTH;
    static qreal LIST_ITEM_CHILD_INDENT;

    Canvas *mCanvas;
    qreal mViewedTop = 0.;
    qreal mViewedBottom;
    MainWindow *mMainWindow;
};
#include <QPainter>
class ChangeWidthWidget : public QWidget
{
    Q_OBJECT
public:
    ChangeWidthWidget(BoxesList *boxesList, QWidget *parent = 0);

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
    BoxesList *mBoxesList;
};

#endif // BOXESLIST_H
