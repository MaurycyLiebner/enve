// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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

#include "sculptbrush.h"
#include "pointhelpers.h"

SculptBrush::SculptBrush(const qreal expRadius,
                         const qreal hardness,
                         const qreal opacity,
                         const qreal value) :
    mExpRadius(expRadius), mHardness(hardness),
    mOpacity(opacity), mValue(value) {
    updateRadius();
    updateSlopeAndOffset();
}

SculptBrush::SculptBrush(const QMatrix &transform,
                         const SculptBrush &other) :
    SculptBrush(other) {
    map(transform);
}

void SculptBrush::incExpRadius(const qreal inc) {
    mExpRadius += inc;
    updateRadius();
}

void SculptBrush::setHardness(const qreal hardness) {
    mHardness = hardness;
    updateSlopeAndOffset();
}

void SculptBrush::setOpacity(const qreal opacity) {
    mOpacity = opacity;
}

void SculptBrush::setValue(const qreal value) {
    mValue = value;
}

void SculptBrush::setColor(const QColor &color) {
    mColor = color;
}

QPointF SculptBrush::closestOuterPoint(const QPointF &pos) const {
    return mPos2 + scalePointToNewLen(pos - mPos2, mRadius);
}

QPointF SculptBrush::displacement() const {
    return mPos2 - mPos1;
}

qreal SculptBrush::influence(const QPointF &pos) const {
    const qreal dist = pointToLen(mPos1 - pos);
    if(dist > mRadius) return 0;
    const qreal distPow2 = std::pow(dist/mRadius, 2);
    const bool inSeg1 = distPow2 <= mHardness;
    const qreal slope =  inSeg1 ? mSeg1Slope  : mSeg2Slope;
    const qreal offset = inSeg1 ? mSeg1Offset : mSeg2Offset;
    const qreal opa = offset + distPow2*slope;
    return opa*mOpacity*mPressure;
}

void SculptBrush::pressAt(const QPointF &pos, const qreal pressure) {
    mPos1 = pos;
    mPos2 = pos;
    mPressure = pressure;
}

void SculptBrush::moveTo(const QPointF &pos, const qreal pressure) {
    mPos1 = mPos2;
    mPos2 = pos;
    mPressure = pressure;
}

void SculptBrush::updateRadius() {
    mRadius = std::exp(mExpRadius);
}

void SculptBrush::updateSlopeAndOffset() {
    mSeg1Slope = -(1/mHardness - 1);
    mSeg1Offset = 1;
    mSeg2Offset = mHardness/(1 - mHardness);
    mSeg2Slope = -mHardness/(1 - mHardness);
}

void SculptBrush::map(const QMatrix &transform) {
    mPos1 = transform.map(mPos1);
    mPos2 = transform.map(mPos2);
}
