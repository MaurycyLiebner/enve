#include "brushsettings.h"
#include "GUI/BrushWidgets/brushselectionwidget.h"

BrushSettings::BrushSettings() : StaticComplexAnimator("brush settings") {
    ca_addChildAnimator(mWidthCurve);
    ca_addChildAnimator(mPressureCurve);
    ca_addChildAnimator(mSpacingCurve);
    ca_addChildAnimator(mTimeCurve);

    mWidthCurve->setCurrentValue(qCubicSegment1D(1));
    mPressureCurve->setCurrentValue(qCubicSegment1D(1));
    mSpacingCurve->setCurrentValue(qCubicSegment1D(1));
    mTimeCurve->setCurrentValue(qCubicSegment1D(1));
}

void BrushSettings::writeProperty(QIODevice * const dst) const {
    StaticComplexAnimator::writeProperty(dst);
    gWrite(dst, mBrush ? mBrush->getCollectionName() : "");
    gWrite(dst, mBrush ? mBrush->getBrushName() : "");
}

void BrushSettings::readProperty(QIODevice * const src) {
    StaticComplexAnimator::readProperty(src);
    const QString brushCollection = gReadString(src);
    const QString brushName = gReadString(src);
    mBrush = BrushSelectionWidget::sGetBrush(brushCollection, brushName);
}
