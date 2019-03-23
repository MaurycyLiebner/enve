#ifndef BOXSCROLLWIDGETVISIBLEPART_H
#define BOXSCROLLWIDGETVISIBLEPART_H

#include <QWidget>
#include "OptimalScrollArea/scrollwidgetvisiblepart.h"
#include "singlewidgettarget.h"
#include "framerange.h"

class BoxSingleWidget;
class DurationRectangleMovable;
class Key;
class KeysView;

class BoxScrollWidgetVisiblePart : public ScrollWidgetVisiblePart {
    Q_OBJECT
public:
    explicit BoxScrollWidgetVisiblePart(ScrollWidget * const parent);

    QWidget *createNewSingleWidget();
    void paintEvent(QPaintEvent *);
    void drawKeys(QPainter * const p,
                  const qreal &pixelsPerFrame,
                  const FrameRange &viewedFrameRange);
    Key *getKeyAtPos(const int &pressX,
                          const int &pressY,
                          const qreal &pixelsPerFrame,
                          const int &minViewedFrame);
    void getKeysInRect(QRectF selectionRect,
                       const qreal &pixelsPerFrame,
                       QList<Key *> &listKeys);

    void updateDropTarget();
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
    BoxSingleWidget *getBSWAtPos(const int &yPos) const;
    int getIdAtPos(const int &yPos) const;
    BoxSingleWidget *getLastVisibleBSW() const;
protected:
    void dropEvent(QDropEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
private:
    bool mDragging = false;

    QLine mCurrentDragLine;
    int mLastDragMoveY;

    QTimer *mScrollTimer = nullptr;
    KeysView *mKeysView = nullptr;
    struct Dragged {
        SingleWidgetAbstraction * fPtr;
        enum Type { BOX,
                    RASTER_EFFECT, RASTER_GPU_EFFECT, PATH_EFFECT,
                    NONE } fType;

        bool isValid() const {
            return fType != NONE && fPtr;
        }

        void reset() {
            fPtr = nullptr;
            fType = NONE;
        }

    };
    Dragged mCurrentlyDragged;
    struct DropTarget {
        SingleWidgetAbstraction * fTargetParent;
        int fTargetId;

        bool drop(const Dragged& dragged);

        bool isValid() const {
            return fTargetId != -1 && fTargetParent;
        }

        void reset() {
            fTargetParent = nullptr;
            fTargetId = -1;
        }
    };
    DropTarget mDropTarget;

    enum DropType {
        DROP_NONE = 0,
        DROP_ABOVE = 1,
        DROP_INTO = 2,
        DROP_BELOW = 4
    };
    typedef int DropTypes;
    DropTypes dropOnSWTSupported(SingleWidgetTarget const * const swtUnderMouse) const;
    DropTypes dropOnBSWSupported(const BoxSingleWidget * const bswUnderMouse) const;
    DropTarget getClosestDropTarget(const int &yPos) const;

    void updateDraggedFromMimeData(const QMimeData * const mimeData);
    bool droppingSupported(const SingleWidgetAbstraction * const targetAbs,
                           const int &idInTarget) const;
    void updateDragLine();
signals:

public slots:
    void scrollUp();
    void scrollDown();
};

#endif // BOXSCROLLWIDGETVISIBLEPART_H
