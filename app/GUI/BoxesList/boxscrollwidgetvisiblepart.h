#ifndef BOXSCROLLWIDGETVISIBLEPART_H
#define BOXSCROLLWIDGETVISIBLEPART_H

#include <QWidget>
#include "OptimalScrollArea/scrollwidgetvisiblepart.h"
#include "singlewidgettarget.h"

class BoxSingleWidget;
class DurationRectangleMovable;
class Key;
class KeysView;

class BoxScrollWidgetVisiblePart : public ScrollWidgetVisiblePart {
    Q_OBJECT
public:
    explicit BoxScrollWidgetVisiblePart(ScrollWidget *parent = nullptr);

    QWidget *createNewSingleWidget();
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
                       QList<Key *> &listKeys);

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
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);

    bool mDragging = false;

    int mCurrentDragPosId = 0;
    int mLastDragMoveY;

    SWT_TargetTypes mLastDragMoveTargetTypes;
    QTimer *mScrollTimer = nullptr;
    KeysView *mKeysView = nullptr;

signals:

public slots:
    void scrollUp();
    void scrollDown();
};

#endif // BOXSCROLLWIDGETVISIBLEPART_H
