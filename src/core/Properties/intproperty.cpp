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

#include "intproperty.h"

IntProperty::IntProperty(const QString &name) : Property(name) {}

void IntProperty::setValueRange(const int minValue,
                                const int maxValue) {
    mMinValue = minValue;
    mMaxValue = maxValue;
}

void IntProperty::setCurrentValue(const int value) {
    const int newValue = qMin(qMax(value, mMinValue), mMaxValue);
    if(newValue == mValue) return;

    mValue = newValue;
    prp_afterWholeInfluenceRangeChanged();

    emit valueChangedSignal(mValue);
}

void IntProperty::prp_startTransform() {
    if(mTransformed) return;

    //mSavedCurrentValue = mCurrentValue;
    mTransformed = true;
}

void IntProperty::prp_finishTransform() {
    if(mTransformed) {
//        addUndoRedo(new ChangeQrealAnimatorValue(mSavedCurrentValue,
//                                                 mCurrentValue,
//                                                 this) );
        mTransformed = false;

        emit prp_finishTransform();
    }
}

bool IntProperty::SWT_isIntProperty() const { return true; }

void IntProperty::prp_writeProperty(eWriteStream& dst) const {
    dst << mValue;
}

void IntProperty::prp_readProperty(eReadStream& src) {
    src >> mValue;
}

int IntProperty::getValue() {
    return mValue;
}

int IntProperty::getMaxValue() { return mMaxValue; }

int IntProperty::getMinValue() { return mMinValue; }
