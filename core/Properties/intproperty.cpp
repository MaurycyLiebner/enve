#include "intproperty.h"

IntProperty::IntProperty(const QString &name) : Property(name) {}

void IntProperty::setValueRange(const int &minValue,
                                const int &maxValue) {
    mMinValue = minValue;
    mMaxValue = maxValue;
}

void IntProperty::setCurrentValue(const int &value) {
    int newValue = qMin(qMax(value, mMinValue), mMaxValue);
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

        prp_callFinishUpdater();
    }
}

bool IntProperty::SWT_isIntProperty() const { return true; }

int IntProperty::getValue() {
    return mValue;
}

int IntProperty::getMaxValue() { return mMaxValue; }

int IntProperty::getMinValue() { return mMinValue; }
