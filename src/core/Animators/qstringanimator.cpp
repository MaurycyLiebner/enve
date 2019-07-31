#include "qstringanimator.h"
#include "undoredo.h"

QStringAnimator::QStringAnimator(const QString &name) :
    SteppedAnimator<QString>(name) {}
