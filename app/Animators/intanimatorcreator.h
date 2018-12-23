#ifndef INTANIMATORCREATOR_H
#define INTANIMATORCREATOR_H
#include "intanimator.h"
#include "Properties/propertycreator.h"
#include "glhelpers.h"

struct IntAnimatorCreator : public PropertyCreator {
    friend class StdSelfRef;
    int fIniVal;
    int fMinVal;
    int fMaxVal;
    int fStep;

    qsptr<Property> create() const {
        return SPtrCreate(IntAnimator)(
                    fIniVal, fMinVal, fMaxVal, fStep, fName);
    }

    GLint getUniformLocation(QGL33c * const gl, const GLuint& program) const {
        return gl->glGetUniformLocation(program, fName.toLatin1());
    }
private:
    IntAnimatorCreator(const int &iniVal,
                       const int &minVal,
                       const int &maxVal,
                       const int &prefferdStep,
                       const QString& name) : PropertyCreator(name) {
        fIniVal = iniVal;
        fMinVal = minVal;
        fMaxVal = maxVal;
        fStep = prefferdStep;
    }
};
#endif // INTANIMATORCREATOR_H
