// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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

#include "sculptpathbox.h"
#include "RasterEffects/rastereffectcollection.h"
#include "sculptpathboxrenderdata.h"
#include "Animators/transformanimator.h"
#include "Animators/qpointfanimator.h"
#include "Private/document.h"

SculptPathBox::SculptPathBox() : BoundingBox(eBoxType::sculptPath) {
    rename("Sculpt Path");
    mBrushWidth = enve::make_shared<QrealAnimator>(1, 0, 999, 1, "width");
    mPath = enve::make_shared<SculptPathCollection>();
    mFillSettings = enve::make_shared<FillSettingsAnimator>(this);

    ca_prependChild(mRasterEffectsAnimators.data(), mFillSettings);
    ca_prependChild(mRasterEffectsAnimators.data(), mBrushWidth);
    ca_prependChild(mRasterEffectsAnimators.data(), mPath);

    setStrokeBrush(Document::sInstance->fOutlineBrush);
}

void SculptPathBox::setStrokeBrush(SimpleBrushWrapper * const brush) {
    if(mBrush == brush) return;
    {
        UndoRedo ur;
        const stdptr<SimpleBrushWrapper> oldValue = mBrush;
        const stdptr<SimpleBrushWrapper> newValue = brush;
        ur.fUndo = [this, oldValue]() {
            setStrokeBrush(oldValue);
        };
        ur.fRedo = [this, newValue]() {
            setStrokeBrush(newValue);
        };
        prp_addUndoRedo(ur);
    }
    mBrush = brush;
    prp_afterWholeInfluenceRangeChanged();

    emit brushChanged(brush);
}

void SculptPathBox::strokeWidthAction(const QrealAction &action) {
    action.apply(mBrushWidth.get());
}

void SculptPathBox::setupRenderData(const qreal relFrame,
                                    BoxRenderData * const data,
                                    Canvas * const scene) {
    BoundingBox::setupRenderData(relFrame, data, scene);
    const auto sculptData = static_cast<SculptPathBoxRenderData*>(data);

    sculptData->fWidth = mBrushWidth->getEffectiveValue(relFrame);
    sculptData->fPath = mPath->getPathAtRelFrame(relFrame);
    sculptData->fBrush = mBrush ? mBrush->createDuplicate() : nullptr;
    sculptData->fFillType = mPath->getFillType();

    UpdatePaintSettings &fillSettings = sculptData->fPaintSettings;
    mFillSettings->setupPaintSettings(relFrame, fillSettings);
}

stdsptr<BoxRenderData> SculptPathBox::createRenderData() {
    return enve::make_shared<SculptPathBoxRenderData>(this);
}

#include "typemenu.h"
void SculptPathBox::setupCanvasMenu(PropertyMenu * const menu) {
    if(menu->hasActionsForType<SculptPathBox>()) return;
    menu->addedActionsForType<SculptPathBox>();
    BoundingBox::setupCanvasMenu(menu);
    PropertyMenu::PlainSelectedOp<SculptPathBox> op = [](SculptPathBox * box) {
        box->applyCurrentTransform();
    };
    menu->addSeparator();
    menu->addPlainAction("Apply Transformation", op);
}

void SculptPathBox::readBoundingBox(eReadStream &src) {
    BoundingBox::readBoundingBox(src);

    SimpleBrushWrapper* brush; src >> brush;
    setStrokeBrush(brush);
}

void SculptPathBox::writeBoundingBox(eWriteStream& dst) const {
    BoundingBox::writeBoundingBox(dst);

    dst << mBrush.data();
}

void SculptPathBox::setPath(const SkPath &path) {
    mPath->prp_startTransform();
    mPath->loadSkPath(path);
    mPath->prp_finishTransform();
}

void SculptPathBox::sculptStarted() {
    mPath->prp_startTransform();
}

void SculptPathBox::sculpt(const SculptTarget target,
                           const SculptMode mode,
                           const SculptBrush &brush) {
    mPath->sculpt(target, mode, brush);
}

void SculptPathBox::sculptCanceled() {
    mPath->prp_cancelTransform();
}

void SculptPathBox::sculptFinished() {
    mPath->prp_finishTransform();
}

void SculptPathBox::applyCurrentTransform() {
    prp_pushUndoRedoName("Apply Transform");
    mNReasonsNotToApplyUglyTransform++;
    const auto transform = mTransformAnimator->getRotScaleShearTransform();
    mPath->applyTransform(transform);
    getFillSettings()->applyTransform(transform);
    mTransformAnimator->startRotScaleShearTransform();
    mTransformAnimator->resetRotScaleShear();
    mTransformAnimator->prp_finishTransform();
    mNReasonsNotToApplyUglyTransform--;
}
