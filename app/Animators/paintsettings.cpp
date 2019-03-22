#include "paintsettings.h"
#include "undoredo.h"
#include "colorhelpers.h"
#include "GUI/GradientWidgets/gradientwidget.h"
#include "GUI/mainwindow.h"
#include "GUI/ColorWidgets/colorvaluerect.h"
#include "PropertyUpdaters/displayedfillstrokesettingsupdater.h"
#include "skia/skqtconversions.h"
#include "skia/skiaincludes.h"
#include "Boxes/pathbox.h"
#include "gradientpoints.h"
#include "PropertyUpdaters/strokewidthupdater.h"
#include "gradient.h"

PaintSettingsAnimator::PaintSettingsAnimator(
        const QString &name,
        GradientPoints * const grdPts,
        PathBox * const parent) :
    PaintSettingsAnimator(name, grdPts, parent, QColor(255, 255, 255),
                          PaintType::FLATPAINT,  nullptr) {
}

PaintSettingsAnimator::PaintSettingsAnimator(
        const QString& name,
        GradientPoints * const grdPts,
        PathBox * const parent,
        const QColor &colorT,
        const PaintType &paintTypeT,
        Gradient* const gradientT) :
    ComplexAnimator(name), mTarget_k(parent) {
    mColor->qra_setCurrentValue(colorT);
    showHideChildrenBeforeChaningPaintType(paintTypeT);
    mPaintType = paintTypeT;
    setGradientVar(gradientT);
    setGradientPoints(grdPts);

    prp_setOwnUpdater(SPtrCreate(DisplayedFillStrokeSettingsUpdater)(parent));
}

void PaintSettingsAnimator::setGradientVar(Gradient* const grad) {
    if(grad == mGradient) return;
    if(mGradient) {
        ca_removeChildAnimator(GetAsSPtr(mGradient, Gradient));
        ca_removeChildAnimator(GetAsSPtr(mGradientPoints, GradientPoints));
        mGradient->removePath(mTarget_k);
    }
    mGradient = grad;
    if(mGradient) {
        ca_addChildAnimator(GetAsSPtr(grad, Gradient));
        ca_addChildAnimator(GetAsSPtr(mGradientPoints, GradientPoints));
        mGradient->addPath(mTarget_k);
    }

    prp_callFinishUpdater();
}

QColor PaintSettingsAnimator::getCurrentColor() const {
    return mColor->getCurrentColor();
}

QColor PaintSettingsAnimator::getColorAtRelFrame(const qreal &relFrame) const {
    return mColor->getColorAtRelFrame(relFrame);
}

const PaintType& PaintSettingsAnimator::getPaintType() const {
    return mPaintType;
}

Gradient *PaintSettingsAnimator::getGradient() const {
    return mGradient.data();
}

void PaintSettingsAnimator::setGradient(Gradient* gradient) {
    if(gradient == mGradient) return;
    setGradientVar(gradient);
    mTarget_k->requestGlobalFillStrokeUpdateIfSelected();
}

void PaintSettingsAnimator::setCurrentColor(const QColor &color) {
    mColor->qra_setCurrentValue(color);
}

void PaintSettingsAnimator::showHideChildrenBeforeChaningPaintType(
        const PaintType &newPaintType) {
    if(mPaintType == FLATPAINT || mPaintType == BRUSHPAINT) {
        ca_removeChildAnimator(mColor);
    }

    if(newPaintType == FLATPAINT || newPaintType == BRUSHPAINT) {
        ca_addChildAnimator(mColor);
    }
}

void PaintSettingsAnimator::setPaintType(const PaintType &paintType) {
    if(paintType == mPaintType) return;

    showHideChildrenBeforeChaningPaintType(paintType);

    mPaintType = paintType;
    mTarget_k->updateDrawGradients();
    mTarget_k->requestGlobalFillStrokeUpdateIfSelected();
    prp_callFinishUpdater();
}

ColorAnimator *PaintSettingsAnimator::getColorAnimator() {
    return mColor.data();
}

void PaintSettingsAnimator::setGradientPoints(GradientPoints* const gradientPoints) {
    mGradientPoints = gradientPoints;
}

OutlineSettingsAnimator::OutlineSettingsAnimator(GradientPoints * const grdPts,
                                                 PathBox * const parent) :
    OutlineSettingsAnimator(grdPts, parent, QColor(0, 0, 0),
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

void OutlineSettingsAnimator::showHideChildrenBeforeChaningPaintType(
        const PaintType &newPaintType) {
    PaintSettingsAnimator::showHideChildrenBeforeChaningPaintType(newPaintType);
    if(getPaintType() == BRUSHPAINT) ca_removeChildAnimator(mBrushSettings);
    if(newPaintType == BRUSHPAINT) ca_addChildAnimator(mBrushSettings);
}

void OutlineSettingsAnimator::setCurrentStrokeWidth(const qreal &newWidth) {
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
        const qreal &relFrame, SkStroke * const stroker) {
    const qreal widthT = mLineWidth->getEffectiveValueAtRelFrame(relFrame);
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

UpdatePaintSettings::UpdatePaintSettings(const QColor &paintColorT,
                                         const PaintType &paintTypeT) {
    fPaintColor = paintColorT;
    fPaintType = paintTypeT;
}

UpdatePaintSettings::UpdatePaintSettings() {}

UpdatePaintSettings::~UpdatePaintSettings() {}

void UpdatePaintSettings::applyPainterSettingsSk(SkPaint *paint) {
    if(fPaintType == GRADIENTPAINT) {
        //p->setBrush(gradient);
        paint->setShader(fGradient);
        paint->setAlpha(255);
    } else if(fPaintType == FLATPAINT) {
        paint->setColor(toSkColor(fPaintColor));
    } else {
        paint->setColor(SkColorSetARGB(0, 0, 0, 0));
    }
}

void UpdatePaintSettings::updateGradient(const QGradientStops &stops,
                                         const QPointF &start,
                                         const QPointF &finalStop,
                                         const bool &linearGradient) {
    int nStops = stops.count();
    SkPoint gradPoints[nStops];
    SkColor gradColors[nStops];
    SkScalar gradPos[nStops];
    const SkScalar xInc = static_cast<SkScalar>(finalStop.x() - start.x());
    const SkScalar yInc = static_cast<SkScalar>(finalStop.y() - start.y());
    SkScalar currX = static_cast<SkScalar>(start.x());
    SkScalar currY = static_cast<SkScalar>(start.y());
    SkScalar currT = 0;
    const SkScalar tInc = 1.f/(nStops - 1);

    for(int i = 0; i < nStops; i++) {
        const QGradientStop &stopT = stops.at(i);
        const QColor col = stopT.second;
        gradPoints[i] = SkPoint::Make(currX, currY);
        gradColors[i] = toSkColor(col);
        gradPos[i] = currT;

        currX += xInc;
        currY += yInc;
        currT += tInc;
    }
    if(linearGradient) {
        fGradient = SkGradientShader::MakeLinear(gradPoints, gradColors,
                                                 gradPos, nStops,
                                                 SkShader::kClamp_TileMode);
    } else {
        const QPointF distPt = finalStop - start;
        const SkScalar radius = static_cast<SkScalar>(
                    qSqrt(distPt.x()*distPt.x() + distPt.y()*distPt.y()));
        fGradient = SkGradientShader::MakeRadial(
                        toSkPoint(start), radius,
                        gradColors, gradPos,
                        nStops, SkShader::kClamp_TileMode);
    }
}

UpdateStrokeSettings::UpdateStrokeSettings(const qreal &width,
                   const QColor &paintColorT,
                   const PaintType &paintTypeT,
                   const QPainter::CompositionMode &outlineCompositionModeT) :
    UpdatePaintSettings(paintColorT, paintTypeT), fOutlineWidth(width) {
    fOutlineCompositionMode = outlineCompositionModeT;
}

UpdateStrokeSettings::UpdateStrokeSettings() {}

void UpdateStrokeSettings::applyPainterSettingsSk(SkPaint *paint) {
    UpdatePaintSettings::applyPainterSettingsSk(paint);
    //canvas->setCompositionMode(outlineCompositionMode);
}
