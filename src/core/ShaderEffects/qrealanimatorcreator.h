#ifndef QREALANIMATORCREATOR_H
#define QREALANIMATORCREATOR_H
#include "Animators/qrealanimator.h"
#include "shaderpropertycreator.h"
#include "glhelpers.h"

struct QrealAnimatorCreator : public ShaderPropertyCreator {
    e_OBJECT
    QrealAnimatorCreator(const qreal iniVal,
                         const qreal minVal,
                         const qreal maxVal,
                         const qreal step,
                         const bool glValue,
                         const QString& name) :
        ShaderPropertyCreator(glValue, name),
        fIniVal(iniVal), fMinVal(minVal),
        fMaxVal(maxVal), fStep(step) {}

    const qreal fIniVal;
    const qreal fMinVal;
    const qreal fMaxVal;
    const qreal fStep;

    qsptr<Property> create() const {
        return enve::make_shared<QrealAnimator>(
                    fIniVal, fMinVal, fMaxVal, fStep, fName);
    }
};
#endif // QREALANIMATORCREATOR_H
