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
    void execOpOnSelectedBoxes(const std::function<void(const QList<T*>&)> &op) {
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
    void execOpOnSelectedPoints(const std::function<void(const QList<T*>&)> &op) {
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
    void execOpOnSelectedProperties(const std::function<void(const QList<T*>&)> &op) {

    }

protected:
    QList<MovablePoint*> mSelectedPoints_d;
    stdptr<MovablePoint> mPressedPoint;

    ConnContextObjList<BoundingBox*> mSelectedBoxes;
};

#endif // CANVASBASE_H
