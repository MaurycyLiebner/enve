#ifndef QSTRINGANIMATOR_H
#define QSTRINGANIMATOR_H
#include "Animators/steppedanimator.h"

typedef KeyT<QString> QStringKey;

class QStringAnimator : public SteppedAnimator<QString> {
    e_OBJECT
protected:
    QStringAnimator(const QString& name);
public:
    bool SWT_isQStringAnimator() const { return true; }
};

#endif // QSTRINGANIMATOR_H
