// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "property.h"
#include "Animators/complexanimator.h"
#include "undoredo.h"
#include "Animators/transformanimator.h"
#include "typemenu.h"
#include "document.h"
#include "canvas.h"

Property::Property(const QString& name) :
    prp_mName(name) {
    connect(this, &Property::prp_ancestorChanged, this, [this]() {
        mParentScene = mParent_k ? mParent_k->mParentScene : nullptr;
    });
}

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
    prp_afterChangedCurrent(UpdateReason::userChange);
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
    if(prp_mInheritedFrameShift == shift) return;
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
    if(mParent_k == parent) return;
    if(mParent_k) {
        disconnect(mParent_k, &Property::prp_ancestorChanged,
                   this, &Property::prp_ancestorChanged);
    }
    mParent_k = parent;
    if(parent) {
        connect(mParent_k, &Property::prp_ancestorChanged,
                this, &Property::prp_ancestorChanged);
    }
    if(mPointsHandler) mPointsHandler->setTransform(getTransformAnimator());
    emit prp_parentChanged(parent);
    emit prp_ancestorChanged();
}

#include "Boxes/boundingbox.h"
void Property::prp_selectionChangeTriggered(const bool shiftPressed) {
    if(!mParentScene) return;
    if(shiftPressed) {
        if(prp_mSelected) {
            mParentScene->removeFromSelectedProps(this);
        } else {
            mParentScene->addToSelectedProps(this);
        }
    } else {
        mParentScene->clearSelectedProps();
        mParentScene->addToSelectedProps(this);
    }
}
