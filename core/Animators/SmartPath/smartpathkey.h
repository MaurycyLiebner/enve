#ifndef SMARTPATHKEY_H
#define SMARTPATHKEY_H
#include "../interpolationkeyt.h"
#include "smartpathcontainer.h"

class SmartPathKey : public InterpolationKeyT<SmartPath> {
    friend class StdSelfRef;
public:
    void save();
    void restore();

    SkPath getPath() const {
        return getValue().getPathAt();
    }
    void assignValue(const SmartPath &value);
protected:
    SmartPathKey(const SmartPath& value, const int relFrame,
                 Animator * const parentAnimator);
    SmartPathKey(Animator * const parentAnimator);
};

#endif // SMARTPATHKEY_H
