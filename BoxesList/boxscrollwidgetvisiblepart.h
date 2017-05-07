#ifndef BOXSCROLLWIDGETVISIBLEPART_H
#define BOXSCROLLWIDGETVISIBLEPART_H

#include <QWidget>
#include "OptimalScrollArea/scrollwidgetvisiblepart.h"
#include "OptimalScrollArea/singlewidgettarget.h"

class BoxSingleWidget;
class QrealKey;
class DurationRectangleMovable;
class Key;
class KeysView;

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
    Key *getKeyAtPos(const int &pressX,
                          const int &pressY,
                          const qreal &pixelsPerFrame,
                          const int &minViewedFrame);
    void getKeysInRect(QRectF selectionRect,
                       qreal pixelsPerFrame,
                       QList<Key *> *listKeys);

    BoxSingleWidget *getClosestsSingleWidgetWithTargetType(
            const SWT_TargetTypes &type, const int &yPos, bool *isBelow);
    void updateDraggingHighlight();
    BoxSingleWidget *getClosestsSingleWidgetWithTargetTypeLookBelow(
            const SWT_TargetTypes &type, const int &yPos, bool *isBelow);
    void stopScrolling();
    DurationRectangleMovable *getRectangleMovableAtPos(
                                        const int &pressX,
                                        const int &pressY,
                                        const qreal &pixelsPerFrame,
                                        const int &minViewedFrame);
    KeysView *getKeysView() {
        return mKeysView;
    }

    void setKeysView(KeysView *keysView) {
        mKeysView = keysView;
    }

protected:
    void dropEvent(QDropEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *);
    void dragMoveEvent(QDragMoveEvent *event);

    QTimer *mScrollTimer = NULL;

    int mLastDragMoveY;
    SWT_TargetTypes mLastDragMoveTargetTypes;

    KeysView *mKeysView = NULL;

    bool mDragging = false;
    int mCurrentDragPosId = 0;

    void dragEnterEvent(QDragEnterEvent *event);
signals:

public slots:
    void scrollUp();
    void scrollDown();
};

#endif // BOXSCROLLWIDGETVISIBLEPART_H
