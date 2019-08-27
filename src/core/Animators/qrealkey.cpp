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

#include "qrealkey.h"
#include "qrealanimator.h"
#include "complexanimator.h"
#include "qrealpoint.h"

QrealKey::QrealKey(const qreal value, const int frame,
                   QrealAnimator * const parentAnimator) :
    GraphKey(frame, parentAnimator) {
    mValue = value;
    setStartValueVar(mValue);
    setEndValueVar(mValue);
}

QrealKey::QrealKey(QrealAnimator * const parentAnimator) :
    QrealKey(0, 0, parentAnimator) { }

void QrealKey::incValue(const qreal incBy) {
    setValue(mValue + incBy);
}

QrealAnimator *QrealKey::getParentQrealAnimator() const {
    return static_cast<QrealAnimator*>(mParentAnimator.data());
}

//bool QrealKey::isNear(qreal frameT, qreal valueT,
//                        qreal pixelsPerFrame, qreal pixelsPerValue) {
//    qreal value = getValue();
//    qreal frame = getFrame();
//    if(qAbs(frameT - frame)*pixelsPerFrame > mRadius) return false;
//    if(qAbs(valueT - value)*pixelsPerValue > mRadius) return false;
//    return true;
//}


qreal QrealKey::getValue() const { return mValue; }

void QrealKey::setValue(const qreal value) {
    const qreal dVal = value - mValue;
    setStartValueVar(mStartPt.getRawYValue() + dVal);
    setEndValueVar(mEndPt.getRawYValue() + dVal);

    mValue = value;
    if(!this->mParentAnimator) return;
    mParentAnimator->anim_updateAfterChangedKey(this);
}

void QrealKey::writeKey(eWriteStream& dst) {
    Key::writeKey(dst);
    dst << mValue;

    dst << mStartEnabled;
    dst.write(&mStartPt, sizeof(ClampedPoint));

    dst << mEndEnabled;
    dst.write(&mEndPt, sizeof(ClampedPoint));
}

void QrealKey::readKey(eReadStream& src) {
    Key::readKey(src);
    src >> mValue;

    src >> mStartEnabled;
    src.read(&mStartPt, sizeof(ClampedPoint));

    src >> mEndEnabled;
    src.read(&mEndPt, sizeof(ClampedPoint));
}

void QrealKey::finishValueTransform() {

}

void QrealKey::startValueTransform() {
    mSavedValue = mValue;
    mStartPt.saveYValue();
    mEndPt.saveYValue();
}

void QrealKey::cancelValueTransform() {
    setValue(mSavedValue);
    setStartValueVar(mStartPt.getRawSavedYValue());
    setEndValueVar(mEndPt.getRawSavedYValue());
}

bool QrealKey::differsFromKey(Key *key) const {
    if(key == this) return false;
    const auto qaKey = static_cast<QrealKey*>(key);
    if(isZero4Dec(qaKey->getValue() - mValue)) {
        if(key->getRelFrame() > mRelFrame) {
            if(qaKey->getStartEnabledForGraph() || mEndEnabled) return true;
        } else {
            if(qaKey->getEndEnabledForGraph() || mStartEnabled) return true;
        }
        return false;
    }
    return true;
}

void QrealKey::changeFrameAndValueBy(const QPointF &frameValueChange) {
    setValue(frameValueChange.y() + mSavedValue);
    GraphKey::changeFrameAndValueBy(frameValueChange);
}
