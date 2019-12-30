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
#include "Private/document.h"
#include "canvas.h"

Property::Property(const QString& name) :
    prp_mName(name) {
    connect(this, &Property::prp_ancestorChanged, this, [this]() {
        mParentScene = mParent_k ? mParent_k->mParentScene : nullptr;
    });
}

void Property::prp_updateCanvasProps() {
    if(mParent_k) mParent_k->prp_updateCanvasProps();
}

void Property::prp_drawCanvasControls(
        SkCanvas * const canvas, const CanvasMode mode,
        const float invScale, const bool ctrlPressed) {
    if(mPointsHandler) {
        bool key;
        if(SWT_isAnimator()) {
            const auto asAnim = static_cast<Animator*>(this);
            key = asAnim->anim_getKeyOnCurrentFrame();
        } else key = false;
        mPointsHandler->drawPoints(canvas, invScale, key, mode, ctrlPressed);
    }
}

void Property::prp_setupTreeViewMenu(PropertyMenu * const menu) {
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

void Property::prp_afterChangedRelRange(const FrameRange &range, const bool clip) {
    const auto absRange = prp_relRangeToAbsRange(range);
    prp_afterChangedAbsRange(absRange, clip);
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
    Q_UNUSED(parentAnimator)
    if(prp_mInheritedFrameShift == shift) return;
    const auto oldRange = prp_absInfluenceRange();
    prp_mInheritedFrameShift = shift;
    const auto newRange = prp_absInfluenceRange();
    prp_afterFrameShiftChanged(oldRange, newRange);
}

void Property::prp_afterFrameShiftChanged(const FrameRange &oldAbsRange,
                                          const FrameRange &newAbsRange) {
    prp_afterChangedAbsRange((newAbsRange + oldAbsRange).adjusted(-1, 1), false);
}

BasicTransformAnimator *Property::getTransformAnimator() const {
    if(mParent_k) return mParent_k->getTransformAnimator();
    return nullptr;
}

QMatrix Property::getTransform() const {
    const auto trans = getTransformAnimator();
    if(trans) return trans->getTotalTransform();
    return QMatrix();
}

void Property::prp_setSelected(const bool selected) {
    if(prp_mSelected == selected) return;
    prp_mSelected = selected;
    emit prp_selectionChanged(selected, QPrivateSignal());
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
    emit prp_nameChanged(newName, QPrivateSignal());
}

bool Property::prp_differencesBetweenRelFrames(
        const int frame1, const int frame2) const {
    return !prp_getIdenticalRelRange(frame1).inRange(frame2);
}

FrameRange Property::prp_absInfluenceRange() const {
    return prp_relRangeToAbsRange(prp_relInfluenceRange());
}

void Property::enabledDrawingOnCanvas() {
    mDrawOnCanvas = true;
    if(mParent_k) mParent_k->prp_updateCanvasProps();
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
    emit prp_parentChanged(parent, QPrivateSignal());
    emit prp_ancestorChanged(QPrivateSignal());
}

bool Property::prp_isParentBoxSelected() const {
    const auto pBox = getFirstAncestor<eBoxOrSound>();
    if(pBox) return pBox->isSelected();
    return false;
}

#include "canvas.h"
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
