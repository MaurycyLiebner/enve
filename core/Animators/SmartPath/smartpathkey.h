#ifndef SMARTPATHKEY_H
#define SMARTPATHKEY_H
#include "../interpolationkeyt.h"
#include "smartpathcontainer.h"

class SmartPathKey : public InterpolationKeyT<SmartPath> {
    friend class StdSelfRef;
public:
    void updateAfterPrevKeyChanged(Key * const prevKey);
    void updateAfterNextKeyChanged(Key * const nextKey);

    void save();
    void restore();

    void assignValue(const SmartPath& value);
protected:
    SmartPathKey(const SmartPath& value, const int &relFrame,
                 Animator * const parentAnimator);
    SmartPathKey(Animator * const parentAnimator);
};

#endif // SMARTPATHKEY_H
