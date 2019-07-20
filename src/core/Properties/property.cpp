#include "property.h"
#include "Animators/complexanimator.h"
#include "PropertyUpdaters/propertyupdater.h"
#include "undoredo.h"
#include "Animators/transformanimator.h"

Property::Property(const QString& name) :
    prp_mName(name) {}

void Property::drawCanvasControls(SkCanvas * const canvas,
                                  const CanvasMode mode,
                                  const SkScalar invScale) {
    if(mPointsHandler) {
        bool key;
        if(SWT_isAnimator()) {
            const auto asAnim = GetAsPtr(this, Animator);
            key = asAnim->anim_getKeyOnCurrentFrame();
        } else key = false;
        mPointsHandler->drawPoints(canvas, invScale, key, mode);
    }
}

void Property::prp_afterChangedAbsRange(const FrameRange &range) {
    prp_callUpdater();
    emit prp_absFrameRangeChanged(range);
}

void Property::prp_afterWholeInfluenceRangeChanged() {
    prp_afterChangedAbsRange(prp_absInfluenceRange());
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

void Property::prp_setParentFrameShift(const int shift,
                                       ComplexAnimator *parentAnimator) {
    Q_UNUSED(parentAnimator);
    prp_mParentFrameShift = shift;
    prp_afterFrameShiftChanged();
}

QMatrix Property::getTransform() const {
    const auto trans = getTransformAnimator();
    if(trans) return trans->getTotalTransform();
    return QMatrix();
}

FrameRange Property::prp_relRangeToAbsRange(const FrameRange& range) const {
    return {prp_relFrameToAbsFrame(range.fMin),
                prp_relFrameToAbsFrame(range.fMax)};
}

FrameRange Property::prp_absRangeToRelRange(const FrameRange& range) const {
    return {prp_absFrameToRelFrame(range.fMin),
            prp_absFrameToRelFrame(range.fMax)};
}

int Property::prp_absFrameToRelFrame(const int absFrame) const {
    if(absFrame == FrameRange::EMIN) return FrameRange::EMIN;
    if(absFrame == FrameRange::EMAX) return FrameRange::EMAX;
    return absFrame - prp_getFrameShift();
}

qreal Property::prp_absFrameToRelFrameF(const qreal absFrame) const {
    if(qRound(absFrame) == FrameRange::EMIN) return FrameRange::EMIN;
    if(qRound(absFrame) == FrameRange::EMAX) return FrameRange::EMAX;
    return absFrame - prp_getFrameShift();
}

int Property::prp_relFrameToAbsFrame(const int relFrame) const {
    if(relFrame == FrameRange::EMIN) return FrameRange::EMIN;
    if(relFrame == FrameRange::EMAX) return FrameRange::EMAX;
    return relFrame + prp_getFrameShift();
}

qreal Property::prp_relFrameToAbsFrameF(const qreal relFrame) const {
    if(qRound(relFrame) == FrameRange::EMIN) return FrameRange::EMIN;
    if(qRound(relFrame) == FrameRange::EMAX) return FrameRange::EMAX;
    return relFrame + prp_getFrameShift();
}

void Property::prp_setName(const QString &newName) {
    if(newName == prp_mName) return;
    prp_mName = newName;
    emit prp_nameChanged(newName);
}

void Property::prp_setUpdater(const stdsptr<PropertyUpdater>& updater) {
    prp_mUpdater = updater;
}

void Property::prp_setInheritedUpdater(const stdsptr<PropertyUpdater>& updater) {
    if(!prp_mOwnUpdater) prp_setUpdater(updater);
}

void Property::prp_setOwnUpdater(const stdsptr<PropertyUpdater>& updater) {
    prp_setUpdater(updater);
    prp_mOwnUpdater = true;
}

void Property::prp_callUpdater() {
    if(prp_mUpdater) prp_mUpdater->update();
}

void Property::prp_callFinishUpdater() {
    if(prp_mUpdater) prp_mUpdater->finishedChange();
}

void Property::enabledDrawingOnCanvas() {
    mDrawOnCanvas = true;
    if(mParent_k) mParent_k->updateCanvasProps();
}

void Property::setPointsHandler(const stdsptr<PointsHandler> &handler) {
    mPointsHandler = handler;
    if(mPointsHandler) {
        enabledDrawingOnCanvas();
        mPointsHandler->setTransform(getTransformAnimator());
    }
}

void Property::addUndoRedo(const stdsptr<UndoRedo>& undoRedo) {
    if(!mParentCanvasUndoRedoStack) return;
    mParentCanvasUndoRedoStack->addUndoRedo(undoRedo);
}

void Property::setParent(ComplexAnimator * const parent) {
    mParent_k = parent;
    if(mPointsHandler) mPointsHandler->setTransform(getTransformAnimator());
}
