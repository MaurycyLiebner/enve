#include "property.h"
#include "Animators/complexanimator.h"

Property::Property() :
    QObject(),
    ConnectedToMainWindow(),
    SingleWidgetTarget(),
    SmartPointerTarget() {

}

void Property::prp_valueChanged() {
    prp_updateAfterChangedAbsFrameRange(INT_MIN, INT_MAX);
}

void Property::prp_updateAfterChangedAbsFrameRange(const int &minFrame,
                                                   const int &maxFrame) {
    if(prp_mParentAnimator == NULL) return;
    prp_mParentAnimator->prp_updateAfterChangedAbsFrameRange(minFrame,
                                                             maxFrame);
}

void Property::prp_setParentAnimator(ComplexAnimator *parentAnimator) {
    prp_mParentAnimator = parentAnimator;
}

void Property::prp_setZValue(const int &oldIndex,
                             const int &newIndex) {
    if(prp_mParentAnimator == NULL) return;
    prp_mParentAnimator->ca_changeChildAnimatorZ(oldIndex, newIndex);
}

QString Property::prp_getName() {
    return prp_mName;
}

int Property::prp_getFrameShift() const {
    return prp_getParentFrameShift();
}

int Property::prp_getParentFrameShift() const {
    if(prp_mParentAnimator == NULL) {
        return 0;
    }
    return prp_mParentAnimator->prp_getFrameShift();
}

int Property::prp_absFrameToRelFrame(
                            const int &absFrame) const {
    return absFrame - prp_getFrameShift();
}

int Property::prp_relFrameToAbsFrame(
                            const int &relFrame) const {
    return relFrame + prp_getFrameShift();
}

int Property::prp_absFrameToParentRelFrame(
                            const int &absFrame) const {
    return absFrame - prp_getParentFrameShift();
}

int Property::prp_parentRelFrameToAbsFrame(
                            const int &relFrame) const {
    return relFrame + prp_getParentFrameShift();
}

void Property::prp_setName(QString newName) {
    prp_mName = newName;
}

void Property::prp_setUpdater(AnimatorUpdater *updater) {
    if(prp_mUpdaterBlocked) return;
    if(prp_mUpdater != NULL) prp_mUpdater->decNumberPointers();
    prp_mUpdater = updater;
    if(prp_mUpdater != NULL) prp_mUpdater->incNumberPointers();
}

void Property::prp_blockUpdater() {
    prp_mUpdaterBlocked = true;
}

void Property::prp_callUpdater() {
    if(prp_mUpdater == NULL) {
        return;
    } else {
        prp_mUpdater->update();
    }
}

void Property::prp_callFinishUpdater() {
    if(prp_mUpdater == NULL) return;
    prp_mUpdater->updateFinal();
}
