#ifndef QREALANIMATORCREATOR_H
#define QREALANIMATORCREATOR_H
#include "Animators/qrealanimator.h"
#include "propertycreator.h"
#include "glhelpers.h"

struct QrealAnimatorCreator : public PropertyCreator {
    e_OBJECT
    qreal fIniVal;
    qreal fMinVal;
    qreal fMaxVal;
    qreal fStep;

    qsptr<Property> create() const {
        return enve::make_shared<QrealAnimator>(
                    fIniVal, fMinVal, fMaxVal, fStep, fName);
    }

    GLint getUniformLocation(QGL33 * const gl, const GLuint& program) const {
        return gl->glGetUniformLocation(program, fName.toLatin1());
    }
private:
    QrealAnimatorCreator(const qreal iniVal,
                         const qreal minVal,
                         const qreal maxVal,
                         const qreal prefferdStep,
                         const QString& name) : PropertyCreator(name) {
        fIniVal = iniVal;
        fMinVal = minVal;
        fMaxVal = maxVal;
        fStep = prefferdStep;
    }
};
#endif // QREALANIMATORCREATOR_H
