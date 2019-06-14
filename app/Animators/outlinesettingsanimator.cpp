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
    mBrushSettings->writeProperty(dst);
    dst->write(rcConstChar(&mCapStyle), sizeof(Qt::PenCapStyle));
    dst->write(rcConstChar(&mJoinStyle), sizeof(Qt::PenJoinStyle));
    dst->write(rcConstChar(&mOutlineCompositionMode),
               sizeof(QPainter::CompositionMode));
}

void OutlineSettingsAnimator::readProperty(QIODevice * const src) {
    PaintSettingsAnimator::readProperty(src);
    mLineWidth->readProperty(src);
    mBrushSettings->readProperty(src);
    src->read(rcChar(&mCapStyle), sizeof(Qt::PenCapStyle));
    src->read(rcChar(&mJoinStyle), sizeof(Qt::PenJoinStyle));
    src->read(rcChar(&mOutlineCompositionMode),
              sizeof(QPainter::CompositionMode));
}

void OutlineSettingsAnimator::showHideChildrenBeforeChaningPaintType(
        const PaintType &newPaintType) {
    PaintSettingsAnimator::showHideChildrenBeforeChaningPaintType(newPaintType);
    if(getPaintType() == BRUSHPAINT) ca_removeChildAnimator(mBrushSettings);
    if(newPaintType == BRUSHPAINT) ca_addChildAnimator(mBrushSettings);
}

void OutlineSettingsAnimator::setCurrentStrokeWidth(const qreal newWidth) {
    mLineWidth->setCurrentBaseValue(newWidth);
}

void OutlineSettingsAnimator::setCapStyle(const Qt::PenCapStyle &capStyle) {
    mCapStyle = capStyle;
    prp_callFinishUpdater();
}

void OutlineSettingsAnimator::setJoinStyle(const Qt::PenJoinStyle &joinStyle) {
    mJoinStyle = joinStyle;
    prp_callFinishUpdater();
}

void OutlineSettingsAnimator::setStrokerSettings(QPainterPathStroker * const stroker) {
    stroker->setWidth(mLineWidth->getCurrentBaseValue());
    stroker->setCapStyle(mCapStyle);
    stroker->setJoinStyle(mJoinStyle);
}

void OutlineSettingsAnimator::setStrokerSettingsSk(SkStroke * const stroker) {
    stroker->setWidth(toSkScalar(mLineWidth->getCurrentBaseValue()));
    stroker->setCap(QCapToSkCap(mCapStyle));
    stroker->setJoin(QJoinToSkJoin(mJoinStyle));
}

void OutlineSettingsAnimator::setStrokerSettingsForRelFrameSk(
        const qreal relFrame, SkStroke * const stroker) {
    const qreal widthT = mLineWidth->getEffectiveValue(relFrame);
    stroker->setWidth(toSkScalar(widthT));
    stroker->setCap(QCapToSkCap(mCapStyle));
    stroker->setJoin(QJoinToSkJoin(mJoinStyle));
}

qreal OutlineSettingsAnimator::getCurrentStrokeWidth() const {
    return mLineWidth->getCurrentBaseValue();
}

Qt::PenCapStyle OutlineSettingsAnimator::getCapStyle() const {
    return mCapStyle;
}

Qt::PenJoinStyle OutlineSettingsAnimator::getJoinStyle() const {
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

