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

#include "smartctrlpoint.h"
#include "smartnodepoint.h"
#include "pointhelpers.h"
#include "Animators/SmartPath/smartpathanimator.h"
#include "Animators/transformanimator.h"

SmartCtrlPoint::SmartCtrlPoint(SmartNodePoint * const parentPoint,
                               const Type& type) :
    NonAnimatedMovablePoint(parentPoint->getTransform(),
                            MovablePointType::TYPE_CTRL_POINT),
    mCtrlType(type), mParentPoint_k(parentPoint) {
    disableSelection();
}

void SmartCtrlPoint::setRelativePos(const QPointF &relPos) {
    NonAnimatedMovablePoint::setRelativePos(relPos);
    if(mCtrlType == C0) mParentPoint_k->c0Moved(getRelativePos());
    else mParentPoint_k->c2Moved(getRelativePos());
}

void SmartCtrlPoint::rotateRelativeToSavedPivot(const qreal rotate) {
    const QPointF savedValue = getSavedRelPos() - mParentPoint_k->getSavedRelPos();
    QMatrix mat;
    mat.rotate(rotate);
    setRelativePos(mat.map(savedValue) + mParentPoint_k->getRelativePos());
}

void SmartCtrlPoint::scale(const qreal sx, const qreal sy) {
    const QPointF savedValue = getSavedRelPos() - mParentPoint_k->getSavedRelPos();
    QMatrix mat;
    mat.scale(sx, sy);
    setRelativePos(mat.map(savedValue) + mParentPoint_k->getRelativePos());
}

bool SmartCtrlPoint::enabled() const {
    if(mCtrlType == C0) return mParentPoint_k->getC0Enabled();
    // if(mCtrlType == C2)
    else return mParentPoint_k->getC2Enabled();
}

void SmartCtrlPoint::startTransform() {
    NonAnimatedMovablePoint::startTransform();
    if(mParentPoint_k->getCtrlsMode() != CTRLS_CORNER) {
        mOtherCtrlPt_cv->NonAnimatedMovablePoint::startTransform();
    }
    mParentPoint_k->getTargetAnimator()->startPathChange();
}

void SmartCtrlPoint::finishTransform() {
    if(mParentPoint_k->getCtrlsMode() != CTRLS_CORNER) {
        mOtherCtrlPt_cv->NonAnimatedMovablePoint::finishTransform();
    }
    mParentPoint_k->getTargetAnimator()->finishPathChange();
}

void SmartCtrlPoint::cancelTransform() {
    NonAnimatedMovablePoint::cancelTransform();
    if(mParentPoint_k->getCtrlsMode() != CTRLS_CORNER) {
        mOtherCtrlPt_cv->NonAnimatedMovablePoint::cancelTransform();
    }
    mParentPoint_k->getTargetAnimator()->cancelPathChange();
}

void SmartCtrlPoint::setOtherCtrlPt(SmartCtrlPoint * const ctrlPt) {
    mOtherCtrlPt_cv = ctrlPt;
}

void SmartCtrlPoint::remove() {
    if(mCtrlType == C0) mParentPoint_k->setC0Enabled(false);
    else mParentPoint_k->setC2Enabled(false);
}

bool SmartCtrlPoint::isVisible(const CanvasMode mode) const {
    if(mParentPoint_k->isDissolved() ||
       mParentPoint_k->isHidden(mode)) return false;
    if(mCtrlType == C0) return mParentPoint_k->getC0Enabled() &&
                               mParentPoint_k->isPrevNormalSelected();
    if(mCtrlType == C2) return mParentPoint_k->getC2Enabled() &&
                               mParentPoint_k->isNextNormalSelected();
    return false;
}
