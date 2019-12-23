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

#include "outlinesettingsanimator.h"
#include "Boxes/pathbox.h"

OutlineSettingsAnimator::OutlineSettingsAnimator(
        GradientPoints * const grdPts,
        PathBox * const parent) :
    PaintSettingsAnimator("outline", grdPts, parent) {

    ca_addChild(mLineWidth);
    connect(mLineWidth.get(), &Property::prp_currentFrameChanged,
            parent, &PathBox::setOutlinePathOutdated);
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
    mCapStyle = capStyle;
    prp_afterWholeInfluenceRangeChanged();
}

void OutlineSettingsAnimator::setJoinStyle(const SkPaint::Join joinStyle) {
    mJoinStyle = joinStyle;
    prp_afterWholeInfluenceRangeChanged();
}

void OutlineSettingsAnimator::setStrokerSettingsSk(SkStroke * const stroker) {
    stroker->setWidth(toSkScalar(mLineWidth->getCurrentBaseValue()));
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

qreal OutlineSettingsAnimator::getCurrentStrokeWidth() const {
    return mLineWidth->getCurrentBaseValue();
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
        const QPainter::CompositionMode &compositionMode) {
    mOutlineCompositionMode = compositionMode;
}

QPainter::CompositionMode OutlineSettingsAnimator::getOutlineCompositionMode() {
    return mOutlineCompositionMode;
}

bool OutlineSettingsAnimator::nonZeroLineWidth() {
    return !isZero4Dec(mLineWidth->getCurrentBaseValue());
}

QrealAnimator *OutlineSettingsAnimator::getLineWidthAnimator() {
    return mLineWidth.data();
}

