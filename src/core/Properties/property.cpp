#include "property.h"
#include "Animators/complexanimator.h"
#include "undoredo.h"
#include "Animators/transformanimator.h"
#include "typemenu.h"
#include "document.h"

Property::Property(const QString& name) :
    prp_mName(name) {}

void Property::drawCanvasControls(SkCanvas * const canvas,
                                  const CanvasMode mode,
                                  const float invScale,
                                  const bool ctrlPressed) {
    if(mPointsHandler) {
        bool key;
        if(SWT_isAnimator()) {
            const auto asAnim = static_cast<Animator*>(this);
            key = asAnim->anim_getKeyOnCurrentFrame();
        } else key = false;
        mPointsHandler->drawPoints(canvas, invScale, key, mode, ctrlPressed);
    }
}

void Property::setupTreeViewMenu(PropertyMenu * const menu) {
    const auto clipboard = Document::sInstance->getPropertyClipboard();
    const bool compat = clipboard && clipboard->compatibleTarget(this);
    menu->addPlainAction("Paste", [this, clipboard]() {
        clipboard->paste(this);
    })->setEnabled(compat);
    menu->addPlainAction("Copy", [this]() {
        const auto clipboard = enve::make_shared<PropertyClipboard>(this);
        Document::sInstance->replaceClipboard(clipboard);
    });
}

void Property::prp_afterChangedAbsRange(const FrameRange &range,
                                        const bool clip) {
    emit prp_currentFrameChanged(UpdateReason::userChange);
    emit prp_absFrameRangeChanged(range, clip);
}

void Property::prp_afterWholeInfluenceRangeChanged() {
    prp_afterChangedAbsRange(prp_absInfluenceRange());
}

const QString &Property::prp_getName() const {
    return prp_mName;
}

int Property::prp_getTotalFrameShift() const {
    return prp_getRelFrameShift() + prp_getInheritedFrameShift();
}

int Property::prp_getInheritedFrameShift() const {
    return prp_mInheritedFrameShift;
}

void Property::prp_setInheritedFrameShift(const int shift,
                                          ComplexAnimator *parentAnimator) {
    Q_UNUSED(parentAnimator);
    const auto oldRange = prp_absInfluenceRange();
    prp_mInheritedFrameShift = shift;
    const auto newRange = prp_absInfluenceRange();
    prp_afterFrameShiftChanged(oldRange, newRange);
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
    return absFrame - prp_getTotalFrameShift();
}

qreal Property::prp_absFrameToRelFrameF(const qreal absFrame) const {
    if(qRound(absFrame) == FrameRange::EMIN) return FrameRange::EMIN;
    if(qRound(absFrame) == FrameRange::EMAX) return FrameRange::EMAX;
    return absFrame - prp_getTotalFrameShift();
}

int Property::prp_relFrameToAbsFrame(const int relFrame) const {
    if(relFrame == FrameRange::EMIN) return FrameRange::EMIN;
    if(relFrame == FrameRange::EMAX) return FrameRange::EMAX;
    return relFrame + prp_getTotalFrameShift();
}

qreal Property::prp_relFrameToAbsFrameF(const qreal relFrame) const {
    if(qRound(relFrame) == FrameRange::EMIN) return FrameRange::EMIN;
    if(qRound(relFrame) == FrameRange::EMAX) return FrameRange::EMAX;
    return relFrame + prp_getTotalFrameShift();
}

void Property::prp_setName(const QString &newName) {
    if(newName == prp_mName) return;
    prp_mName = newName;
    emit prp_nameChanged(newName);
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
