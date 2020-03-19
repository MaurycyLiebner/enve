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

#include "outlinesettingsanimator.h"
#include "Boxes/pathbox.h"
#include "Private/document.h"

OutlineSettingsAnimator::OutlineSettingsAnimator(BoundingBox * const parent) :
    PaintSettingsAnimator("outline", parent) {

    ca_addChild(mLineWidth);
}

void OutlineSettingsAnimator::prp_writeProperty(eWriteStream& dst) const {
    PaintSettingsAnimator::prp_writeProperty(dst);
    mLineWidth->prp_writeProperty(dst);
    mBrushSettings->prp_writeProperty(dst);
    dst.write(&mCapStyle, sizeof(SkPaint::Cap));
    dst.write(&mJoinStyle, sizeof(SkPaint::Join));
    dst.write(&mOutlineCompositionMode, sizeof(QPainter::CompositionMode));
}

void OutlineSettingsAnimator::prp_readProperty(eReadStream& src) {
    PaintSettingsAnimator::prp_readProperty(src);
    mLineWidth->prp_readProperty(src);
    mBrushSettings->prp_readProperty(src);
    src.read(&mCapStyle, sizeof(SkPaint::Cap));
    src.read(&mJoinStyle, sizeof(SkPaint::Join));
    src.read(&mOutlineCompositionMode, sizeof(QPainter::CompositionMode));
}

void OutlineSettingsAnimator::setPaintType(const PaintType paintType) {
    PaintSettingsAnimator::setPaintType(paintType);
    const auto defaultBrush = Document::sInstance->fOutlineBrush;
    if(paintType == BRUSHPAINT && !mBrushSettings->getBrush()) {
        mBrushSettings->setBrush(defaultBrush);
    }
}

void OutlineSettingsAnimator::showHideChildrenBeforeChaningPaintType(
        const PaintType newPaintType) {
    PaintSettingsAnimator::showHideChildrenBeforeChaningPaintType(newPaintType);
    if(getPaintType() == BRUSHPAINT) ca_removeChild(mBrushSettings);
    if(newPaintType == BRUSHPAINT) ca_addChild(mBrushSettings);
}

void OutlineSettingsAnimator::strokeWidthAction(const QrealAction& action) {
    action.apply(mLineWidth.get());
}

void OutlineSettingsAnimator::setCurrentStrokeWidth(const qreal newWidth) {
    mLineWidth->setCurrentBaseValue(newWidth);
}

void OutlineSettingsAnimator::setCapStyle(const SkPaint::Cap capStyle) {
    if(mCapStyle == capStyle) return;
    {
        UndoRedo ur;
        const auto oldValue = mCapStyle;
        const auto newValue = capStyle;
        ur.fUndo = [this, oldValue]() {
            setCapStyle(oldValue);
        };
        ur.fRedo = [this, newValue]() {
            setCapStyle(newValue);
        };
        prp_addUndoRedo(ur);
    }
    mCapStyle = capStyle;
    prp_afterWholeInfluenceRangeChanged();
}

void OutlineSettingsAnimator::setJoinStyle(const SkPaint::Join joinStyle) {
    if(mJoinStyle == joinStyle) return;
    {
        UndoRedo ur;
        const auto oldValue = mJoinStyle;
        const auto newValue = joinStyle;
        ur.fUndo = [this, oldValue]() {
            setJoinStyle(oldValue);
        };
        ur.fRedo = [this, newValue]() {
            setJoinStyle(newValue);
        };
        prp_addUndoRedo(ur);
    }
    mJoinStyle = joinStyle;
    prp_afterWholeInfluenceRangeChanged();
}

void OutlineSettingsAnimator::setStrokerSettingsSk(SkStroke * const stroker) {
    stroker->setWidth(toSkScalar(mLineWidth->getEffectiveValue()));
    stroker->setCap(mCapStyle);
    stroker->setJoin(mJoinStyle);
}

void OutlineSettingsAnimator::setStrokerSettingsForRelFrameSk(
        const qreal relFrame, SkStroke * const stroker) {
    const qreal widthT = mLineWidth->getEffectiveValue(relFrame);
    stroker->setWidth(toSkScalar(widthT));
    stroker->setCap(mCapStyle);
    stroker->setJoin(mJoinStyle);
}

void OutlineSettingsAnimator::duplicateStrokeSettingsNotAnim(
        OutlineSettingsAnimator * const settings) {
    duplicatePaintSettingsNotAnim(settings);
    if(settings) {
        const qreal width = settings->getCurrentStrokeWidth();
        mLineWidth->setCurrentBaseValue(width);
    }
}

void OutlineSettingsAnimator::saveSVG(QDomDocument& doc,
                                      QDomElement& parent,
                                      QDomElement& defs,
                                      const FrameRange& absRange,
                                      const qreal fps, const bool loop) const {
    PaintSettingsAnimator::saveSVG(doc, parent, defs, absRange,
                                   fps, "stroke", loop);
    mLineWidth->saveQrealSVG(doc, parent, defs, absRange, fps,
                             "stroke-width", loop);
}

void OutlineSettingsAnimator::setupStrokeSettings(const qreal relFrame,
                                                  UpdateStrokeSettings &settings) {
    setupPaintSettings(relFrame, settings);
    settings.fOutlineWidth = mLineWidth->getEffectiveValue(relFrame);
}

qreal OutlineSettingsAnimator::getCurrentStrokeWidth() const {
    return mLineWidth->getEffectiveValue();
}

SkPaint::Cap OutlineSettingsAnimator::getCapStyle() const {
    return mCapStyle;
}

SkPaint::Join OutlineSettingsAnimator::getJoinStyle() const {
    return mJoinStyle;
}

QrealAnimator *OutlineSettingsAnimator::getStrokeWidthAnimator() {
    return mLineWidth.data();
}

void OutlineSettingsAnimator::setOutlineCompositionMode(
        const QPainter::CompositionMode compositionMode) {
    mOutlineCompositionMode = compositionMode;
}

QPainter::CompositionMode OutlineSettingsAnimator::getOutlineCompositionMode() {
    return mOutlineCompositionMode;
}

bool OutlineSettingsAnimator::nonZeroLineWidth() {
    return !isZero4Dec(mLineWidth->getEffectiveValue());
}

QrealAnimator *OutlineSettingsAnimator::getLineWidthAnimator() {
    return mLineWidth.data();
}

