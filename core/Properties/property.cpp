#include "property.h"
#include "Animators/complexanimator.h"
#include "PropertyUpdaters/propertyupdater.h"
#include "undoredo.h"

Property::Property(const QString& name) :
    prp_mName(name) {}

void Property::prp_updateAfterChangedAbsFrameRange(const FrameRange &range) {
    prp_currentFrameChanged();
    emit prp_absFrameRangeChanged(range);
}

void Property::prp_updateInfluenceRangeAfterChanged() {
    prp_currentFrameChanged();
    emit prp_updateWholeInfluenceRange();
}

const QString &Property::prp_getName() const {
    return prp_mName;
}

int Property::prp_getFrameShift() const {
    return prp_getRelFrameShift() + prp_getParentFrameShift();
}

int Property::prp_getParentFrameShift() const {
    return prp_mParentFrameShift;
}

void Property::prp_setParentFrameShift(const int &shift,
                                       ComplexAnimator *parentAnimator) {
    Q_UNUSED(parentAnimator);
    prp_mParentFrameShift = shift;
}

FrameRange Property::prp_relRangeToAbsRange(const FrameRange& range) const {
    return {prp_relFrameToAbsFrame(range.fMin),
            prp_relFrameToAbsFrame(range.fMax)};
}

FrameRange Property::prp_absRangeToRelRange(const FrameRange& range) const {
    return {prp_absFrameToRelFrame(range.fMin),
            prp_absFrameToRelFrame(range.fMax)};
}

int Property::prp_absFrameToRelFrame(const int &absFrame) const {
    if(absFrame == FrameRange::EMIN) return FrameRange::EMIN;
    if(absFrame == FrameRange::EMAX) return FrameRange::EMAX;
    return absFrame - prp_getFrameShift();
}

qreal Property::prp_absFrameToRelFrameF(const qreal &absFrame) const {
    if(qRound(absFrame) == FrameRange::EMIN) return FrameRange::EMIN;
    if(qRound(absFrame) == FrameRange::EMAX) return FrameRange::EMAX;
    return absFrame - prp_getFrameShift();
}

int Property::prp_relFrameToAbsFrame(const int &relFrame) const {
    if(relFrame == FrameRange::EMIN) return FrameRange::EMIN;
    if(relFrame == FrameRange::EMAX) return FrameRange::EMAX;
    return relFrame + prp_getFrameShift();
}

qreal Property::prp_relFrameToAbsFrameF(const qreal &relFrame) const {
    if(qRound(relFrame) == FrameRange::EMIN) return FrameRange::EMIN;
    if(qRound(relFrame) == FrameRange::EMAX) return FrameRange::EMAX;
    return relFrame + prp_getFrameShift();
}

void Property::prp_setName(const QString &newName) {
    prp_mName = newName;
}

void Property::prp_setUpdater(const stdsptr<PropertyUpdater>& updater) {
    prp_mUpdater = updater;
}

void Property::prp_setInheritedUpdater(const stdsptr<PropertyUpdater>& updater) {
    if(!prp_mOwnUpdater) prp_setUpdater(updater);
}

void Property::prp_setOwnUpdater(const stdsptr<PropertyUpdater>& updater) {
    prp_mUpdater = updater;
    prp_mOwnUpdater = true;
}

void Property::prp_currentFrameChanged() {
    if(prp_mUpdater) prp_mUpdater->update();
}

void Property::prp_callFinishUpdater() {
    if(prp_mUpdater) prp_mUpdater->finishedChange();
}

void Property::addUndoRedo(const stdsptr<UndoRedo>& undoRedo) {
    if(!mParentCanvasUndoRedoStack) return;
    mParentCanvasUndoRedoStack->addUndoRedo(undoRedo);
}

void Property::graphUpdateAfterKeysChanged() {
    //mMainWindow->getKeysView()->graphUpdateAfterKeysChanged();
}

void Property::graphScheduleUpdateAfterKeysChanged() {
    //mMainWindow->getKeysView()->scheduleGraphUpdateAfterKeysChanged();
}

void Property::setParent(ComplexAnimator * const parent) {
    mParent = parent;
}
