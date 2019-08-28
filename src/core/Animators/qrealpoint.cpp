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

#include "qrealpoint.h"
#include "graphkey.h"

QrealPoint::QrealPoint(QrealPointType type,
                       GraphKey * const parentKey,
                       const qreal radius) {
    mRadius = radius;
    mType = type;
    mParentKey = parentKey;
}

qreal QrealPoint::getRelFrame() {
    if(mType == KEY_POINT) return mParentKey->getRelFrame();
    if(mType == START_POINT) return mParentKey->getStartFrame();
    /*if(mType == END_POINT)*/ return mParentKey->getEndFrame();
}

void QrealPoint::setRelFrame(const qreal frame) {
    if(mType == KEY_POINT) {
        return mParentKey->setRelFrameAndUpdateParentAnimator(qRound(frame));
    }
    if(mType == START_POINT) return mParentKey->setStartFrame(frame);
    if(mType == END_POINT) return mParentKey->setEndFrame(frame);
}

void QrealPoint::setAbsFrame(const qreal absFrame) {
    setRelFrame(mParentKey->absFrameToRelFrameF(absFrame));
}

void QrealPoint::startTransform() {
    mParentKey->startFrameAndValueTransform();
}

void QrealPoint::finishTransform() {
    mParentKey->finishFrameAndValueTransform();
}

qreal QrealPoint::getValue() {
    if(mType == KEY_POINT) return mParentKey->getValueForGraph();
    if(mType == START_POINT) return mParentKey->getStartValue();
    /*if(mType == END_POINT)*/ return mParentKey->getEndValue();
}

void QrealPoint::setValue(const qreal value) {
    if(mType == KEY_POINT) return mParentKey->setValueForGraph(value);
    if(mType == START_POINT) return mParentKey->setStartValueForGraph(value);
    if(mType == END_POINT) return mParentKey->setEndValueForGraph(value);
}

bool QrealPoint::isSelected() {
    if(mType == KEY_POINT) return mParentKey->isSelected();
    return mIsSelected;
}

bool QrealPoint::isNear(const qreal absFrame, const qreal value,
                        const qreal pixelsPerFrame, const qreal pixelsPerValue) {
    if(qAbs(getAbsFrame() - absFrame)*pixelsPerFrame > mRadius) return false;
    if(qAbs(getValue() - value)*pixelsPerValue > mRadius) return false;
    return true;
}

void QrealPoint::moveBy(const qreal dFrame, const qreal dValue) {
    if(mType == KEY_POINT) {
        mParentKey->changeFrameAndValueBy({dFrame, dValue});
    } else if(mType == START_POINT) {
        const auto& pt = mParentKey->startPt();
        const auto relTo = pt.getRawSavedValue();
        mParentKey->setStartFrameVar(relTo.x() + dFrame);
        mParentKey->setStartValueVar(relTo.y() + dValue);
    } else if(mType == END_POINT) {
        const auto& pt = mParentKey->endPt();
        const auto relTo = pt.getRawSavedValue();
        mParentKey->setEndFrameVar(relTo.x() + dFrame);
        mParentKey->setEndValueVar(relTo.y() + dValue);
    }
}

void QrealPoint::moveTo(const qreal frame, const qreal value) {
    setAbsFrame(frame);
    setValue(value);
    if(isKeyPt()) return;
    mParentKey->updateCtrlFromCtrl(mType);
}

qreal QrealPoint::getAbsFrame() {
    return mParentKey->relFrameToAbsFrameF(getRelFrame());
}

QPointF QrealPoint::getSavedFrameAndValue() const {
    const QPointF keySaved(mParentKey->getRelFrame(),
                           mParentKey->getValueForGraph());
    if(mType == KEY_POINT) {
        return keySaved;
    } else if(mType == START_POINT) {
        const auto& pt = mParentKey->startPt();
        return pt.getClampedSavedValue(keySaved);
    } else /*if(mType == END_POINT)*/ {
        const auto& pt = mParentKey->endPt();
        return pt.getClampedSavedValue(keySaved);
    }
}

qreal QrealPoint::getSavedFrame() const {
    return getSavedFrameAndValue().x();
}

qreal QrealPoint::getSavedValue() const {
    return getSavedFrameAndValue().y();
}

void QrealPoint::draw(QPainter * const p, const QColor &paintColor) {
    const QPointF center(getAbsFrame(), getValue());

    p->setBrush(Qt::black);
    if(mHovered) gDrawCosmeticEllipse(p, center, mRadius + 1, mRadius + 1);
    else gDrawCosmeticEllipse(p, center, mRadius, mRadius);

    p->setBrush(paintColor);
    if(!isSelected()) p->setBrush(paintColor.lighter());

    gDrawCosmeticEllipse(p, center, mRadius - 1, mRadius - 1);
}

void QrealPoint::setSelected(const bool selected) {
    mIsSelected = selected;
}

bool QrealPoint::isEnabled() {
    if(isKeyPt()) return true;
    if(isStartPt()) return mParentKey->getStartEnabledForGraph();
    /*if(isEndPoint())*/ return mParentKey->getEndEnabledForGraph();
}
