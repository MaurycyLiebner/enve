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

#include "pointshandler.h"

PointsHandler::PointsHandler() {}

void PointsHandler::insertPt(const int id, const stdsptr<MovablePoint> &pt) {
    pt->setTransform(mTrans);
    mPts.insert(id, pt);
}

void PointsHandler::appendPt(const stdsptr<MovablePoint> &pt) {
    return insertPt(mPts.count(), pt);
}

MovablePoint *PointsHandler::getPointAtAbsPos(const QPointF &absPos,
                                              const CanvasMode mode,
                                              const qreal invScale) {
    for(int i = mPts.count() - 1; i >= 0; i--) {
        const auto& pt = mPts.at(i);
        const auto at = pt->getPointAtAbsPos(absPos, mode, invScale);
        if(at) return at;
    }
    return nullptr;
}

void PointsHandler::addAllPointsToSelection(const MovablePoint::Adder& adder,
                                            const CanvasMode mode) const {
    for(const auto& pt : mPts) {
        if(pt->isSelected() || pt->isHidden(mode)) continue;
        if(pt->selectionEnabled()) {
            pt->select();
            adder(pt.get());
        }
    }
}

void PointsHandler::addInRectForSelection(const QRectF &absRect,
                                          const MovablePoint::Adder& adder,
                                          const CanvasMode mode) const {
    for(const auto& pt : mPts) {
        if(!pt->selectionEnabled()) continue;
        if(pt->isSelected() || pt->isHidden(mode)) continue;
        pt->rectPointsSelection(absRect, mode, adder);
    }
}

void PointsHandler::drawPoints(SkCanvas * const canvas,
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

void PointsHandler::removeLast() { mPts.removeLast(); }

void PointsHandler::removeAt(const int id) { mPts.removeAt(id); }

void PointsHandler::clear() { mPts.clear(); }

bool PointsHandler::isEmpty() const { return mPts.isEmpty(); }

int PointsHandler::count() const { return mPts.count(); }

void PointsHandler::move(const int from, const int to) {
    mPts.move(from, to);
}

BasicTransformAnimator *PointsHandler::transform() const {
    return mTrans;
}

void PointsHandler::setTransform(BasicTransformAnimator * const trans) {
    if(trans == mTrans) return;
    for(const auto& pt : mPts) pt->setTransform(trans);
    mTrans = trans;
}
