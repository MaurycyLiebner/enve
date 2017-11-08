#include "intproperty.h"

IntProperty::IntProperty()  :
    Property() {

}

void IntProperty::setValueRange(const int &minValue,
                                const int &maxValue) {
    mMinValue = minValue;
    mMaxValue = maxValue;
}

void IntProperty::setCurrentValue(const int &value,
                                  const bool &saveUndoRedo,
                                  const bool &finish,
                                  const bool &callUpdater) {
    int newValue = qMin(qMax(value, mMinValue), mMaxValue);
    if(newValue == mValue) return;

    if(saveUndoRedo) {
        prp_startTransform();
        mValue = newValue;
        emit valueChangedSignal(mValue);
        prp_finishTransform();
        return;
    }

    mValue = newValue;
    if(finish) {
        prp_updateInfluenceRangeAfterChanged();
    }

    emit valueChangedSignal(mValue);
    if(callUpdater) {
        prp_callUpdater();
    }
}

bool IntProperty::SWT_isIntProperty() { return true; }

int IntProperty::getValue() {
    return mValue;
}

int IntProperty::getMaxValue() { return mMaxValue; }

int IntProperty::getMinValue() { return mMinValue; }
