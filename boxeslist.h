#ifndef BOXESLIST_H
#define BOXESLIST_H

#include <QWidget>

const qreal LIST_ITEM_HEIGHT = 20;
const qreal LIST_ITEM_MAX_WIDTH = 250;
const qreal LIST_ITEM_CHILD_INDENT = 20;

class QrealKey;

class Canvas;

class MainWindow;

class BoxesList : public QWidget
{
    Q_OBJECT
public:
    explicit BoxesList(MainWindow *mainWindow, QWidget *parent = 0);
    void scheduleRepaint();
    void repaintIfNeeded();

    int getMinViewedFrame();
    int getMaxViewedFrame();
    qreal getPixelsPerFrame();

    void updatePixelsPerFrame();

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

    void addKeyToSelection(QrealKey *key);
    void removeKeyFromSelection(QrealKey *key);
    void clearKeySelection();
    void selectKeysInSelectionRect();
    bool processFilteredKeyEvent(QKeyEvent *event);
protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *e);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
signals:

public slots:
    void setFramesRange(int startFrame, int endFrame);
private:
    void deleteSelectedKeys();

    QrealKey *mLastPressedKey = NULL;
    bool mFirstMove = false;
    int mMoveDFrame = 0;
    QPointF mLastPressPos;
    bool mMovingKeys = false;
    bool mSelectingKeys = false;
    QRectF mSelectionRect;

    bool mRepaintScheduled = false;
    Canvas *mCanvas;
    QRectF mViewedRect;
    MainWindow *mMainWindow;
    QList<QrealKey*> mSelectedKeys;

    int mMinViewedFrame = 0;
    int mMaxViewedFrame = 50;
    qreal mPixelsPerFrame;
};

#endif // BOXESLIST_H
