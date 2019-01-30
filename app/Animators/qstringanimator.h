#ifndef QSTRINGANIMATOR_H
#define QSTRINGANIMATOR_H
#include "Animators/steppedanimator.h"

typedef KeyT<QString> QStringKey;

class QStringAnimator : public SteppedAnimator<QString> {
    friend class SelfRef;
public:
    bool SWT_isQStringAnimator() const { return true; }
protected:
    QStringAnimator(const QString& name);
};

#endif // QSTRINGANIMATOR_H
