#include "expressionsourcebase.h"
#include "Animators/qrealanimator.h"

ExpressionSourceBase::ExpressionSourceBase(QrealAnimator * const parent) :
    ExpressionValue(false), mParent(parent) {}


bool ExpressionSourceBase::isValid() const
{ return mSource; }

ConnContext &ExpressionSourceBase::setSource(
        QrealAnimator * const source) {
    return mSource.assign(source);
}

QrealAnimator *ExpressionSourceBase::parent() const
{ return mParent; }

QrealAnimator *ExpressionSourceBase::source() const
{ return mSource; }
