#include "outlinesettingsanimator.h"
#include "PropertyUpdaters/strokewidthupdater.h"

OutlineSettingsAnimator::OutlineSettingsAnimator(GradientPoints * const grdPts,
                                                 PathBox * const parent) :
    OutlineSettingsAnimator(grdPts, parent, Qt::black,
                            PaintType::FLATPAINT, nullptr) {}

OutlineSettingsAnimator::OutlineSettingsAnimator(
        GradientPoints * const grdPts,
        PathBox * const parent,
        const QColor &color,
        const PaintType &paintType,
        Gradient* const gradient) :
    PaintSettingsAnimator("outline", grdPts, parent,
                          color, paintType, gradient) {

    ca_addChildAnimator(mLineWidth);
    mLineWidth->prp_setOwnUpdater(SPtrCreate(StrokeWidthUpdater)(parent));
}

void OutlineSettingsAnimator::writeProperty(QIODevice * const dst) const {
    PaintSettingsAnimator::writeProperty(dst);
    mLineWidth->writeProperty(dst);
    dst->write(rcConstChar(&mCapStyle), sizeof(SkPaint::Cap));
    dst->write(rcConstChar(&mJoinStyle), sizeof(SkPaint::Join));
    dst->write(rcConstChar(&mOutlineCompositionMode),
               sizeof(QPainter::CompositionMode));
}

void OutlineSettingsAnimator::readProperty(QIODevice * const src) {
    PaintSettingsAnimator::readProperty(src);
    mLineWidth->readProperty(src);
    src->read(rcChar(&mCapStyle), sizeof(SkPaint::Cap));
    src->read(rcChar(&mJoinStyle), sizeof(SkPaint::Join));
    src->read(rcChar(&mOutlineCompositionMode),
              sizeof(QPainter::CompositionMode));
}

void OutlineSettingsAnimator::strokeWidthAction(const QrealAction& action) {
    action.apply(mLineWidth.get());
}

void OutlineSettingsAnimator::setCurrentStrokeWidth(const qreal newWidth) {
    mLineWidth->setCurrentBaseValue(newWidth);
}

void OutlineSettingsAnimator::setCapStyle(const SkPaint::Cap capStyle) {
    mCapStyle = capStyle;
    prp_callFinishUpdater();
}

void OutlineSettingsAnimator::setJoinStyle(const SkPaint::Join joinStyle) {
    mJoinStyle = joinStyle;
    prp_callFinishUpdater();
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

