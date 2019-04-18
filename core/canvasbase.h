#ifndef CANVASBASE_H
#define CANVASBASE_H
#include <functional>
#include <QList>
class Property;
class BoundingBox;

#include "MovablePoints/movablepoint.h"

class CanvasBase {
protected:
    CanvasBase();
public:
    template <class T = BoundingBox>
    void execOpOnSelectedBoxes(const std::function<void(QList<T*>)> &op) {
        QList<T*> all;
        for(const auto& box : mSelectedBoxes) {
            const auto boxT = dynamic_cast<T*>(box.data());
            if(boxT) all << boxT;
        }
        op(all);
    }

    template <class T = BoundingBox>
    void execOpOnSelectedBoxes(const std::function<void(T*)> &op) {
        for(const auto& box : mSelectedBoxes) {
            const auto boxT = dynamic_cast<T*>(box.data());
            if(boxT) op(boxT);
        }
    }

    template <class T = MovablePoint>
    void execOpOnSelectedPoints(const std::function<void(QList<T*>)> &op) {
        QList<T*> all;
        for(const auto& pt : mSelectedPoints_d) {
            const auto ptT = dynamic_cast<T*>(pt.data());
            if(ptT) all << ptT;
        }
        op(all);
    }

    template <class T = MovablePoint>
    void execOpOnSelectedPoints(const std::function<void(T*)> &op) {
        if(mLastPressedPoint) {
            if(!mLastPressedPoint->selectionEnabled()) {
                const auto ptT = dynamic_cast<T*>(mLastPressedPoint.data());
                if(ptT) {
                    op(ptT);
                    //if(ptT->selectionEnabled()) addPointToSelection(ptT);
                    return;
                }
            }
        }
        for(const auto& pt : mSelectedPoints_d) {
            const auto ptT = dynamic_cast<T*>(pt.data());
            if(ptT) {
                op(ptT);
                //if(!ptT->selectionEnabled()) removePointFromSelection(ptT);
            }
        }
    }

    template <class T = Property>
    void execOpOnSelectedProperties(const std::function<void(T*)> &op) {

    }

    template <class T = Property>
    void execOpOnSelectedProperties(const std::function<void(QList<T*>)> &op) {

    }

protected:
    QList<stdptr<MovablePoint>> mSelectedPoints_d;
    stdptr<MovablePoint> mLastPressedPoint;

    QList<qptr<BoundingBox>> mSelectedBoxes;
};

#endif // CANVASBASE_H
