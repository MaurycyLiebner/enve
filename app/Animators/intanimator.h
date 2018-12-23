#ifndef INTANIMATOR_H
#define INTANIMATOR_H
#include "Animators/qrealanimator.h"

class IntAnimator : public QrealAnimator {
    friend class SelfRef;
public:
    int getCurrentIntValue() const;
    void setCurrentIntValue(const int &value, const bool &saveUndoRedo,
                            const bool &finish = false);
    void setIntValueRange(const int &minVal, const int &maxVal);

    bool SWT_isIntAnimator() { return true; }
    int getCurrentIntValueAtRelFrame(const int &relFrame) const;
    int getCurrentIntValueAtRelFrameF(const qreal &relFrame) const;
    int getCurrentIntEffectiveValueAtRelFrameF(const qreal &relFrame) const;
protected:
    IntAnimator(const QString& name);
    IntAnimator(const int &iniVal,
                const int &minVal,
                const int &maxVal,
                const int &prefferdStep,
                const QString& name);
};

#endif // INTANIMATOR_H
