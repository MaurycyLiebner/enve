#ifndef BOXESLISTWIDGET_H
#define BOXESLISTWIDGET_H

#include <QWidget>
#include "boundingbox.h"

class WidgetContainer : public QWidget
{
    Q_OBJECT
public:
    WidgetContainer(QWidget *parent = 0);

    void setTopWidget(QWidget *widget);
    void addChildWidget(QWidget *widget);
    void setDetailsVisible(bool visible);
protected:
    QVBoxLayout *mAllLayout;
    QWidget *mDetailsWidget;
    QVBoxLayout *mChildWidgetsLayout;
};

class QrealAnimatorItemWidget : public QWidget
{
    Q_OBJECT
public:
    QrealAnimatorItemWidget(QrealAnimator *target, QWidget *parent = 0);
    virtual void draw(QPainter *p);

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);

    void drawKeys(QPainter *p, qreal pixelsPerFrame,
                  int animatorTop, int maxY,
                  int minViewedFrame, int maxViewedFrame);

    QrealAnimator *getTargetAnimator() { return mTargetAnimator; }
protected:
    QrealAnimator *mTargetAnimator;
};

class ComplexAnimatorItemWidget : public QrealAnimatorItemWidget
{
    Q_OBJECT
public:
    ComplexAnimatorItemWidget(QrealAnimator *target,
                              QWidget *parent = 0);
    void draw(QPainter *p);

    void mousePressEvent(QMouseEvent *event);

    void setDetailsVisibile(bool bT);
signals:
    void detailsVisibilityChanged(bool);
private:
    bool mDetailsVisible = false;
};

class ComplexAnimatorItemWidgetContainer : public WidgetContainer
{
    Q_OBJECT
public:
    ComplexAnimatorItemWidgetContainer(QrealAnimator *target,
                                       QWidget *parent = 0);

    QrealAnimator *getTargetAnimator() { return mTargetAnimatorWidget->getTargetAnimator(); }
    void drawKeys(QPainter *p, qreal pixelsPerFrame,
                  int animatorTop, int maxY,
                  int minViewedFrame, int maxViewedFrame);
    QrealKey *getKeyAtPos(qreal pressX, qreal pressY,
                          qreal pixelsPerFrame, int maxY,
                          int animatorTop, int minViewedFrame);
    void getKeysInRect(QRectF selectionRect, int viewedTop,
                       qreal pixelsPerFrame, int minViewedFrame,
                       QList<QrealKey*> *listKeys);
public slots:
    void addChildAnimator(QrealAnimator *animator);
    void removeChildAnimator(QrealAnimator *animator);
private:
    QrealAnimatorItemWidget *mTargetAnimatorWidget;
    QList<ComplexAnimatorItemWidgetContainer*> mChildWidgets;
};

class BoxItemWidget : public QWidget
{
    Q_OBJECT
public:
    BoxItemWidget(BoundingBox *target, QWidget *parent = 0);

    void paintEvent(QPaintEvent *);

    bool detailsVisible() { return mDetailsVisible; }

    void setDetailsVisibile(bool bT);

    BoundingBox *getTargetBox() { return mTargetBox; }

    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void rename();

    void drawKeys(QPainter *p, qreal pixelsPerFrame,
                  int containerTop, int maxY,
                  int minViewedFrame, int maxViewedFrame);
signals:
    void detailsVisibilityChanged(bool);
private:
    BoundingBox *mTargetBox = NULL;
    bool mDetailsVisible = false;
};

class BoxItemWidgetContainer : public WidgetContainer
{
    Q_OBJECT
public:
    BoxItemWidgetContainer(BoundingBox *target, QWidget *parent = 0);


    BoundingBox *getTargetBox() { return mTargetBoxWidget->getTargetBox(); }

    void drawKeys(QPainter *p, qreal pixelsPerFrame,
                  int containerTop, int maxY,
                  int minViewedFrame, int maxViewedFrame);
    QrealKey *getKeyAtPos(qreal pressX, qreal pressY, qreal pixelsPerFrame, int maxY,
                          int containerTop, int minViewedFrame);
    void getKeysInRect(QRectF selectionRect, int containerTop,
                       qreal pixelsPerFrame, int minViewedFrame,
                       QList<QrealKey*> *listKeys);
public slots:
    void addAnimatorWidgetForAnimator(QrealAnimator *animator);
    void removeAnimatorWidgetForAnimator(QrealAnimator *animator);
private:
    void addAnimatorWidget(ComplexAnimatorItemWidgetContainer *widget);
    void removeAnimatorWidget(ComplexAnimatorItemWidgetContainer *widget);

    BoxItemWidget *mTargetBoxWidget = NULL;
    QList<ComplexAnimatorItemWidgetContainer*> mAnimatorsContainers;
};

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
