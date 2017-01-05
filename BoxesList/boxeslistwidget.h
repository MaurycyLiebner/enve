#ifndef BOXESLISTWIDGET_H
#define BOXESLISTWIDGET_H

#include <QWidget>
#include "Boxes/boundingbox.h"

class BoxesListWidget : public QWidget
{
    Q_OBJECT
public:
    BoxesListWidget(QWidget *parent = 0);

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
    static bool mStaticPixmapsLoaded;
    static void loadStaticPixmaps();


    static int getListItemHeight() { return LIST_ITEM_HEIGHT; }
    static int getListItemMaxWidth() { return LIST_ITEM_MAX_WIDTH; }
    static int getListItemChildIndent() { return LIST_ITEM_CHILD_INDENT; }
    static void setListItemMaxWidth(int widthT) { LIST_ITEM_MAX_WIDTH = widthT; }

    void drawKeys(QPainter *p, qreal pixelsPerFrame, int viewedTop,
                  int minViewedFrame, int maxViewedFrame);
    QrealKey *getKeyAtPos(int pressX, int pressY, qreal pixelsPerFrame,
                          int viewedTop, int minViewedFrame);
    void getKeysInRect(QRectF selectionRect, int viewedTop,
                       qreal pixelsPerFrame, int minViewedFrame,
                       QList<QrealKey*> *listKeys);
private:
    static int LIST_ITEM_HEIGHT;
    static int LIST_ITEM_MAX_WIDTH;
    static int LIST_ITEM_CHILD_INDENT;

    QList<BoxItemWidgetContainer*> mBoxWidgetContainers;
    QVBoxLayout *mBoxesLayout;
signals:

public slots:
    void addItemForBox(BoundingBox *box);
    void removeItemForBox(BoundingBox *box);
private slots:
    void changeItemZ(int from, int to);
};

#endif // BOXESLISTWIDGET_H
