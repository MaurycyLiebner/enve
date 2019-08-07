#ifndef SMARTPATHKEY_H
#define SMARTPATHKEY_H
#include "../interpolationkeyt.h"
#include "smartpath.h"

class SmartPathKey : public InterpolationKeyT<SmartPath> {
    e_OBJECT
protected:
    SmartPathKey(const SmartPath& value, const int relFrame,
                 Animator * const parentAnimator);
    SmartPathKey(Animator * const parentAnimator);
public:
    void save();
    void restore();

    SkPath getPath() const {
        return getValue().getPathAt();
    }
    void assignValue(const SmartPath &value);
};

#endif // SMARTPATHKEY_H
