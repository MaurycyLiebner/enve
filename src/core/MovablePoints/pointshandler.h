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

#ifndef POINTSHANDLER_H
#define POINTSHANDLER_H
#include "movablepoint.h"
#include "../smartPointers/stdselfref.h"
class BoxTransformAnimator;
enum class CanvasMode : short;

class PointsHandler : public StdSelfRef {
    e_OBJECT
protected:
    PointsHandler();
public:
    void insertPt(const int id, const stdsptr<MovablePoint>& pt) {
        pt->setTransform(mTrans);
        mPts.insert(id, pt);
    }

    void appendPt(const stdsptr<MovablePoint>& pt) {
        return insertPt(mPts.count(), pt);
    }

    MovablePoint *getPointAtAbsPos(const QPointF &absPos,
                                   const CanvasMode mode,
                                   const qreal invScale) {
        for(int i = mPts.count() - 1; i >= 0; i--) {
            const auto& pt = mPts.at(i);
            const auto at = pt->getPointAtAbsPos(absPos, mode, invScale);
            if(at) return at;
        }
        return nullptr;
    }

    void addAllPointsToSelection(QList<MovablePoint*> &selection,
                                 const CanvasMode mode) {
        for(const auto& pt : mPts) {
            if(pt->isSelected() || pt->isHidden(mode)) continue;
            if(pt->selectionEnabled()) {
                pt->select();
                selection << pt.get();
            }
        }
    }

    void addInRectForSelection(const QRectF &absRect,
                               QList<MovablePoint*> &selection,
                               const CanvasMode mode) const {
        for(const auto& pt : mPts) {
            if(!pt->selectionEnabled()) continue;
            if(pt->isSelected() || pt->isHidden(mode)) continue;
            pt->rectPointsSelection(absRect, mode, selection);
        }
    }

    void drawPoints(SkCanvas * const canvas,
                    const float invScale,
                    const bool keyOnCurrentFrame,
                    const CanvasMode mode,
                    const bool ctrlPressed) const {
        for(int i = mPts.count() - 1; i >= 0; i--) {
            const auto& pt = mPts.at(i);
            if(pt->isVisible(mode))
                pt->drawSk(canvas, mode, invScale,
                           keyOnCurrentFrame, ctrlPressed);
        }
    }

    template <class T>
    T* getPointWithId(const int id) const {
        if(id < 0) return nullptr;
        if(id >= mPts.count()) return nullptr;
        return static_cast<T*>(mPts.at(id).get());
    }

    void removeLast() { mPts.removeLast(); }
    void removeAt(const int id) { mPts.removeAt(id); }
    void clear() { mPts.clear(); }

    bool isEmpty() const { return mPts.isEmpty(); }
    int count() const { return mPts.count(); }

    void move(const int from, const int to) {
        mPts.move(from, to);
    }

    BasicTransformAnimator * transform() const {
        return mTrans;
    }

    void setTransform(BasicTransformAnimator * const trans) {
        if(trans == mTrans) return;
        for(const auto& pt : mPts) pt->setTransform(trans);
        mTrans = trans;
    }
//protected:
//    QList<stdsptr<MovablePoint>>::const_iterator begin() const {
//        return mPts.begin();
//    }

//    QList<stdsptr<MovablePoint>>::const_iterator end() const {
//        return mPts.end();
//    }
private:
    BasicTransformAnimator * mTrans = nullptr;
    QList<stdsptr<MovablePoint>> mPts;
};

#endif // POINTSHANDLER_H
