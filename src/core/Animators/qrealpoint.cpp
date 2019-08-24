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

QrealPoint::QrealPoint(const QrealPointType& type,
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

void QrealPoint::startFrameTransform() {
    if(mType == KEY_POINT) return mParentKey->startFrameTransform();
}

void QrealPoint::finishFrameTransform() {
    if(mType == KEY_POINT) return mParentKey->finishFrameTransform();
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

bool QrealPoint::isNear(const qreal frameT,
                        const qreal valueT,
                        const qreal pixelsPerFrame,
                        const qreal pixelsPerValue) {
    const qreal value = getValue();
    const qreal frame = getAbsFrame();
    if(qAbs(frameT - frame)*pixelsPerFrame > mRadius) return false;
    if(qAbs(valueT - value)*pixelsPerValue > mRadius) return false;
    return true;
}

void QrealPoint::moveTo(const qreal frameT, const qreal valueT) {
    setAbsFrame(frameT);
    setValue(valueT);
    if(isKeyPoint()) return;
    mParentKey->updateCtrlFromCtrl(mType);
}

qreal QrealPoint::getAbsFrame() {
    return mParentKey->relFrameToAbsFrameF(getRelFrame());
}

void QrealPoint::draw(QPainter * const p, const QColor &paintColor) {
    const QPointF center(getAbsFrame(), getValue());

    p->setBrush(Qt::black);
    if(mHovered)
        gDrawCosmeticEllipse(p, center, mRadius + 1, mRadius + 1);
    else
        gDrawCosmeticEllipse(p, center, mRadius, mRadius);

    p->setBrush(paintColor);
    if(!isSelected()) {
        p->setBrush(paintColor.lighter());
    }/* else {
        drawCosmeticEllipse(p, center,
                            mRadius*0.5 - 1.,
                            mRadius*0.5 - 1.);
    }*/
    gDrawCosmeticEllipse(p, center, mRadius - 1, mRadius - 1);
}

void QrealPoint::setSelected(const bool bT) {
    if(mType == KEY_POINT) {
        Q_ASSERT(false); // key selection handled differently
        mParentKey->setSelected(bT);
    } else {
        mIsSelected = bT;
    }
}

bool QrealPoint::isKeyPoint() { return mType == KEY_POINT; }

bool QrealPoint::isStartPoint() { return mType == START_POINT; }

bool QrealPoint::isEndPoint() { return mType == END_POINT; }

bool QrealPoint::isEnabled() {
    if(isKeyPoint()) return true;
    if(isStartPoint()) return mParentKey->getStartEnabledForGraph();
    /*if(isEndPoint())*/ return mParentKey->getEndEnabledForGraph();
}

GraphKey *QrealPoint::getParentKey() {
    return mParentKey;
}
