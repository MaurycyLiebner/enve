#ifndef BOXSCROLLWIDGETVISIBLEPART_H
#define BOXSCROLLWIDGETVISIBLEPART_H

#include <QWidget>
#include "OptimalScrollArea/scrollwidgetvisiblepart.h"
class QrealKey;

class BoxScrollWidgetVisiblePart : public ScrollWidgetVisiblePart
{
    Q_OBJECT
public:
    explicit BoxScrollWidgetVisiblePart(ScrollWidget *parent = 0);


    SingleWidget *createNewSingleWidget();
    void paintEvent(QPaintEvent *);
    void drawKeys(QPainter *p,
                  const qreal &pixelsPerFrame,
                  const int &minViewedFrame,
                  const int &maxViewedFrame);
    QrealKey *getKeyAtPos(const int &pressX,
                          const int &pressY,
                          const qreal &pixelsPerFrame,
                          const int &minViewedFrame);
    void getKeysInRect(QRectF selectionRect,
                       qreal pixelsPerFrame,
                       const int &minViewedFrame,
                       QList<QrealKey *> *listKeys);

protected:
    void dropEvent(QDropEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *);
    void dragMoveEvent(QDragMoveEvent *event);

    QTimer *mScrollTimer = NULL;

    bool mDragging = false;
    int mCurrentDragPosId = 0;

    void dragEnterEvent(QDragEnterEvent *event);
signals:

public slots:
    void scrollUp();
    void scrollDown();
};

#endif // BOXSCROLLWIDGETVISIBLEPART_H
