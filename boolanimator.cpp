#include "boolanimator.h"

BoolAnimator::BoolAnimator() : IntAnimator()
{

}

bool BoolAnimator::getCurrentBoolValue() {
    return getCurrentIntValue() == 1;
}

void BoolAnimator::setCurrentBoolValue(bool bT, bool finish)
{
    setCurrentIntValue((bT ? 1 : 0), finish);
}
