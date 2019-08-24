// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef BOXSCROLLWIDGETVISIBLEPART_H
#define BOXSCROLLWIDGETVISIBLEPART_H

#include <QWidget>
#include "OptimalScrollArea/scrollwidgetvisiblepart.h"
#include "singlewidgettarget.h"
#include "framerange.h"

class BoxSingleWidget;
class TimelineMovable;
class Key;
class KeysView;
class Canvas;

class BoxScroller : public ScrollWidgetVisiblePart {
public:
    explicit BoxScroller(ScrollWidget * const parent);

    QWidget *createNewSingleWidget();
    void paintEvent(QPaintEvent *);
    void drawKeys(QPainter * const p,
                  const qreal pixelsPerFrame,
                  const FrameRange &viewedFrameRange);
    Key *getKeyAtPos(const int pressX,
                          const int pressY,
                          const qreal pixelsPerFrame,
                          const int minViewedFrame);
    void getKeysInRect(QRectF selectionRect,
                       const qreal pixelsPerFrame,
                       QList<Key *> &listKeys);

    void updateDropTarget();

    void stopScrolling();
    void scrollUp();
    void scrollDown();

    TimelineMovable *getRectangleMovableAtPos(
                                        const int pressX,
                                        const int pressY,
                                        const qreal pixelsPerFrame,
                                        const int minViewedFrame);
    KeysView *getKeysView() const {
        return mKeysView;
    }

    Canvas* currentScene() const {
        return mCurrentScene;
    }

    void setCurrentScene(Canvas* const scene) {
        mCurrentScene = scene;
    }

    void setKeysView(KeysView *keysView) {
        mKeysView = keysView;
    }
protected:
    void dropEvent(QDropEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
private:
    enum class DropType {
        none, on, into
    };

    struct DropTarget {
        SWT_Abstraction * fTargetParent;
        int fTargetId;
        DropType fDropType;

        bool isValid() const {
            return fTargetParent && fDropType != DropType::none;
        }

        void reset() {
            fTargetParent = nullptr;
            fDropType = DropType::none;
        }
    };

    DropTarget getClosestDropTarget(const int yPos);

    bool tryDropIntoAbs(SWT_Abstraction * const abs,
                        const int idInAbs, DropTarget &dropTarget);

    Canvas* mCurrentScene = nullptr;

    QRect mCurrentDragRect;
    int mLastDragMoveY;

    QTimer *mScrollTimer = nullptr;
    KeysView *mKeysView = nullptr;

    const QMimeData* mCurrentMimeData = nullptr;

    DropTarget mDropTarget{nullptr, 0, DropType::none};
};

#endif // BOXSCROLLWIDGETVISIBLEPART_H
