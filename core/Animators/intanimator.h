#ifndef INTANIMATOR_H
#define INTANIMATOR_H
#include "Animators/qrealanimator.h"

class IntAnimator : public QrealAnimator {
    friend class SelfRef;
protected:
    IntAnimator(const QString& name);
    IntAnimator(const int &iniVal,
                const int &minVal,
                const int &maxVal,
                const int &prefferdStep,
                const QString& name);
public:
    int getCurrentIntValue() const;
    void setCurrentIntValue(const int &value);
    void setIntValueRange(const int &minVal, const int &maxVal);

    bool SWT_isIntAnimator() const { return true; }
    int getCurrentIntValueAtRelFrame(const qreal &relFrame) const;
    int getCurrentIntEffectiveValueAtRelFrame(const qreal &relFrame) const;

    static qsptr<IntAnimator> sCreateSeed() {
        const auto result = SPtrCreate(IntAnimator)("seed");
        result->setIntValueRange(0, 9999);
        result->setCurrentIntValue(qrand() % 9999);
        return result;
    }
};

#endif // INTANIMATOR_H
