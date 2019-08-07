#ifndef INTANIMATORCREATOR_H
#define INTANIMATORCREATOR_H
#include "Animators/intanimator.h"
#include "shaderpropertycreator.h"
#include "glhelpers.h"

struct IntAnimatorCreator : public ShaderPropertyCreator {
    e_OBJECT
    IntAnimatorCreator(const int iniVal, const int minVal,
                       const int maxVal, const int step,
                       const bool glValue,
                       const QString& name) :
        ShaderPropertyCreator(glValue, name),
        fIniVal(iniVal), fMinVal(minVal),
        fMaxVal(maxVal), fStep(step) {}

    const int fIniVal;
    const int fMinVal;
    const int fMaxVal;
    const int fStep;

    qsptr<Property> create() const {
        return enve::make_shared<IntAnimator>(
                    fIniVal, fMinVal, fMaxVal, fStep, fName);
    }
};
#endif // INTANIMATORCREATOR_H
