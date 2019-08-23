#include "outlinesettingsanimator.h"
#include "Boxes/pathbox.h"

OutlineSettingsAnimator::OutlineSettingsAnimator(GradientPoints * const grdPts,
                                                 PathBox * const parent) :
    OutlineSettingsAnimator(grdPts, parent, Qt::black,
                            PaintType::FLATPAINT, nullptr) {}

OutlineSettingsAnimator::OutlineSettingsAnimator(
        GradientPoints * const grdPts,
        PathBox * const parent,
        const QColor &color,
        const PaintType paintType,
        Gradient* const gradient) :
    PaintSettingsAnimator("outline", grdPts, parent,
                          color, paintType, gradient) {

    ca_addChild(mLineWidth);
    connect(mLineWidth.get(), &Property::prp_currentFrameChanged,
            parent, &PathBox::setOutlinePathOutdated);
}

void OutlineSettingsAnimator::writeProperty(eWriteStream& dst) const {
    PaintSettingsAnimator::writeProperty(dst);
    mLineWidth->writeProperty(dst);
    dst.write(&mCapStyle, sizeof(SkPaint::Cap));
    dst.write(&mJoinStyle, sizeof(SkPaint::Join));
    dst.write(&mOutlineCompositionMode, sizeof(QPainter::CompositionMode));
}

void OutlineSettingsAnimator::readProperty(eReadStream& src) {
    PaintSettingsAnimator::readProperty(src);
    mLineWidth->readProperty(src);
    src.read(&mCapStyle, sizeof(SkPaint::Cap));
    src.read(&mJoinStyle, sizeof(SkPaint::Join));
    src.read(&mOutlineCompositionMode, sizeof(QPainter::CompositionMode));
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

