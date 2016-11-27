#include "intanimator.h"

IntAnimator::IntAnimator() : QrealAnimator()
{

}

int IntAnimator::getCurrentIntValue() const
{
    return qRound(getCurrentValue());
}

void IntAnimator::setCurrentIntValue(int value, bool finish)
{
    setCurrentValue(value, finish);
}
