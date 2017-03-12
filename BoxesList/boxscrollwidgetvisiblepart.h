#ifndef BOXSCROLLWIDGETVISIBLEPART_H
#define BOXSCROLLWIDGETVISIBLEPART_H

enum SWT_Type : short;

#include <QWidget>
#include "OptimalScrollArea/scrollwidgetvisiblepart.h"

class BoxSingleWidget;
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
                       QList<QrealKey *> *listKeys);

    BoxSingleWidget *getClosestsSingleWidgetWithTargetType(
            const SWT_Type &type, const int &yPos, bool *isBelow);
    void updateDraggingHighlight();
    BoxSingleWidget *getClosestsSingleWidgetWithTargetTypeLookBelow(
            const SWT_Type &type, const int &yPos, bool *isBelow);
    void stopScrolling();
protected:
    void dropEvent(QDropEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *);
    void dragMoveEvent(QDragMoveEvent *event);

    QTimer *mScrollTimer = NULL;

    int mLastDragMoveY;
    SWT_Type mLastDragMoveTargetType;

    bool mDragging = false;
    int mCurrentDragPosId = 0;

    void dragEnterEvent(QDragEnterEvent *event);
signals:

public slots:
    void scrollUp();
    void scrollDown();
};

#endif // BOXSCROLLWIDGETVISIBLEPART_H
