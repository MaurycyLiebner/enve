#include "property.h"
#include "Animators/complexanimator.h"

Property::Property() {

}

void Property::prp_valueChanged() {
    prp_updateAfterChangedAbsFrameRange(INT_MIN, INT_MAX);
}

void Property::prp_updateAfterChangedAbsFrameRange(const int &minFrame,
                                                   const int &maxFrame) {
    emit prp_absFrameRangeChanged(minFrame, maxFrame);
}

const QString &Property::prp_getName() {
    return prp_mName;
}

int Property::prp_getFrameShift() const {
    return prp_getParentFrameShift();
}

int Property::prp_getParentFrameShift() const {
    return prp_mParentFrameShift;
}

void Property::prp_setParentFrameShift(const int &shift) {
    prp_mParentFrameShift = shift;
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

void Property::prp_setName(const QString &newName) {
    prp_mName = newName;
}

void Property::prp_setUpdater(AnimatorUpdater *updater) {
    if(prp_mUpdaterBlocked) return;
    prp_mUpdater = updater;
}

void Property::prp_setBlockedUpdater(AnimatorUpdater *updater) {
    prp_mUpdaterBlocked = false;
    prp_setUpdater(updater);
    prp_blockUpdater();
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
