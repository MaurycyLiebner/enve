#ifndef CANVASBASE_H
#define CANVASBASE_H
#include <functional>
#include <QList>
class Property;
class BoundingBox;

#include "MovablePoints/movablepoint.h"
#include "conncontext.h"

class CanvasBase {
protected:
    CanvasBase();
public:
    template <class T = BoundingBox>
    void execOpOnSelectedBoxes(const std::function<void(QList<T*>)> &op) {
        QList<T*> all;
        for(const auto& box : mSelectedBoxes) {
            const auto boxT = dynamic_cast<T*>(box);
            if(boxT) all << boxT;
        }
        op(all);
    }

    template <class T = BoundingBox>
    void execOpOnSelectedBoxes(const std::function<void(T*)> &op) {
        for(const auto& box : mSelectedBoxes) {
            const auto boxT = dynamic_cast<T*>(box);
            if(boxT) op(boxT);
        }
    }

    template <class T = MovablePoint>
    void execOpOnSelectedPoints(const std::function<void(QList<T*>)> &op) {
        QList<T*> all;
        for(const auto& pt : mSelectedPoints_d) {
            const auto ptT = dynamic_cast<T*>(pt);
            if(ptT) all << ptT;
        }
        op(all);
    }

    template <class T = MovablePoint>
    void execOpOnSelectedPoints(const std::function<void(T*)> &op) {
        if(mPressedPoint) {
            if(!mPressedPoint->selectionEnabled()) {
                const auto ptT = dynamic_cast<T*>(mPressedPoint.data());
                if(ptT) {
                    op(ptT);
                    //if(ptT->selectionEnabled()) addPointToSelection(ptT);
                    return;
                }
            }
        }
        for(const auto& pt : mSelectedPoints_d) {
            const auto ptT = dynamic_cast<T*>(pt);
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
    QList<MovablePoint*> mSelectedPoints_d;
    stdptr<MovablePoint> mPressedPoint;

    ConnContextObjList<BoundingBox*> mSelectedBoxes;
};

#endif // CANVASBASE_H
