#include "paintsettings.h"
#include "undoredo.h"
#include "colorhelpers.h"
#include "GUI/GradientWidgets/gradientwidget.h"
#include "GUI/mainwindow.h"
#include "GUI/ColorWidgets/colorvaluerect.h"
#include "PropertyUpdaters/gradientupdater.h"
#include "PropertyUpdaters/displayedfillstrokesettingsupdater.h"
#include "skia/skqtconversions.h"
#include "skia/skiaincludes.h"
#include "Boxes/pathbox.h"
#include "gradientpoints.h"
#include "PropertyUpdaters/strokewidthupdater.h"

Gradient::Gradient() : ComplexAnimator("gradient") {
    prp_setOwnUpdater(SPtrCreate(GradientUpdater)(this));
}

Gradient::Gradient(const QColor &color1, const QColor &color2) :
    Gradient() {
    addColorToList(color1);
    addColorToList(color2);
    updateQGradientStops(Animator::USER_CHANGE);
}

bool Gradient::isEmpty() const {
    return mColors.isEmpty();
}

void Gradient::prp_startTransform() {
    //savedColors = colors;
}

void Gradient::addColorToList(const QColor &color) {
    auto newColorAnimator = SPtrCreate(ColorAnimator)();
    newColorAnimator->qra_setCurrentValue(color);
    addColorToList(newColorAnimator);
}

void Gradient::addColorToList(const qsptr<ColorAnimator>& newColorAnimator) {
    mColors << newColorAnimator;

    ca_addChildAnimator(newColorAnimator);
}

QColor Gradient::getCurrentColorAt(const int &id) {
    return mColors.at(id)->getCurrentColor();
}

ColorAnimator *Gradient::getColorAnimatorAt(const int &id) {
    return mColors.at(id).get();
}

int Gradient::getColorCount() {
    return mColors.length();
}

QColor Gradient::getLastQGradientStopQColor() {
    return mQGradientStops.last().second;
}

QColor Gradient::getFirstQGradientStopQColor() {
    return mQGradientStops.first().second;
}

QGradientStops Gradient::getQGradientStops() {
    return mQGradientStops;
}

void Gradient::swapColors(const int &id1, const int &id2) {
    ca_swapChildAnimators(mColors.at(id1).get(),
                          mColors.at(id2).get());
    mColors.swap(id1, id2);
    updateQGradientStops(Animator::USER_CHANGE);
}

void Gradient::removeColor(const int &id) {
    removeColor(mColors.at(id));
}

void Gradient::removeColor(const qsptr<ColorAnimator>& color) {
    ca_removeChildAnimator(color);
    emit resetGradientWidgetColorIdIfEquals(this, mColors.indexOf(color));
    mColors.removeOne(color);
    updateQGradientStops(Animator::USER_CHANGE);
}

void Gradient::addColor(const QColor &color) {
    addColorToList(color);
    updateQGradientStops(Animator::USER_CHANGE);
}

void Gradient::replaceColor(const int &id, const QColor &color) {
    mColors.at(id)->qra_setCurrentValue(color);
    updateQGradientStops(Animator::USER_CHANGE);
}

void Gradient::addPath(PathBox * const path) {
    mAffectedPaths << path;
}

void Gradient::removePath(PathBox * const path) {
    mAffectedPaths.removeOne(path);
}

bool Gradient::affectsPaths() {
    return !mAffectedPaths.isEmpty();
}

void Gradient::updatePaths(const UpdateReason &reason) {
    for(const auto& path : mAffectedPaths) {
        path->updateDrawGradients();
        path->scheduleUpdate(reason);
    }
}

//void Gradient::finishTransform() {
//    ComplexAnimator::finishTransform();
//    //addUndoRedo(new ChangeGradientColorsUndoRedo(savedColors, colors, this));
//    //savedColors = colors;
//    callUpdateSchedulers();
//}

void Gradient::startColorIdTransform(const int& id) {
    if(mColors.count() <= id || id < 0) return;
    mColors.at(id)->prp_startTransform();
}

QGradientStops Gradient::getQGradientStopsAtAbsFrame(const qreal &absFrame) {
    QGradientStops stops;
    const qreal inc = 1./(mColors.length() - 1);
    qreal cPos = 0.;
    for(int i = 0; i < mColors.length(); i++) {
        stops.append(QPair<qreal, QColor>(clamp(cPos, 0, 1),
                     mColors.at(i)->getColorAtRelFrame(absFrame)) );
        cPos += inc;
    }
    return stops;
}

void Gradient::updateQGradientStops(const Animator::UpdateReason& reason) {
    mQGradientStops.clear();
    const qreal inc = 1./(mColors.length() - 1);
    qreal cPos = 0;
    for(int i = 0; i < mColors.length(); i++) {
        mQGradientStops.append(QPair<qreal, QColor>(clamp(cPos, 0, 1),
                                    mColors.at(i)->getCurrentColor()) );
        cPos += inc;
    }
    updatePaths(reason);
}


int Gradient::getLoadId() {
    return mLoadId;
}

void Gradient::setLoadId(const int &id) {
    mLoadId = id;
}

PaintSettings::PaintSettings(GradientPoints * const grdPts,
                             PathBox * const parent) :
    PaintSettings(grdPts, parent, QColor(255, 255, 255),
                  PaintType::FLATPAINT,  nullptr) {
}

PaintSettings::PaintSettings(GradientPoints * const grdPts,
                             PathBox * const parent,
                             const QColor &colorT,
                             const PaintType &paintTypeT,
                             Gradient* const gradientT) :
    ComplexAnimator("fill"), mTarget_k(parent) {
    mColor->qra_setCurrentValue(colorT);
    mPaintType = paintTypeT;
    setGradientVar(gradientT);

    ca_addChildAnimator(mColor);
    setGradientPoints(grdPts);

    prp_setOwnUpdater(SPtrCreate(DisplayedFillStrokeSettingsUpdater)(parent));
}

void PaintSettings::setGradientVar(Gradient* const grad) {
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

QColor PaintSettings::getCurrentColor() const {
    return mColor->getCurrentColor();
}

QColor PaintSettings::getColorAtRelFrame(const qreal &relFrame) const {
    return mColor->getColorAtRelFrame(relFrame);
}

const PaintType& PaintSettings::getPaintType() const {
    return mPaintType;
}

Gradient *PaintSettings::getGradient() const {
    return mGradient.data();
}

void PaintSettings::setGradient(Gradient* gradient) {
    if(gradient == mGradient) return;
    setGradientVar(gradient);
    mTarget_k->requestGlobalFillStrokeUpdateIfSelected();
}

void PaintSettings::setCurrentColor(const QColor &color) {
    mColor->qra_setCurrentValue(color);
}

void PaintSettings::showHideChildrenBeforeChaningPaintType(
        const PaintType &newPaintType) {
    if(mPaintType == FLATPAINT || mPaintType == BRUSHPAINT) {
        ca_removeChildAnimator(mColor);
    }

    if(newPaintType == FLATPAINT || newPaintType == BRUSHPAINT) {
        ca_addChildAnimator(mColor);
    }
}

void PaintSettings::setPaintType(const PaintType &paintType) {
    if(paintType == mPaintType) return;

    showHideChildrenBeforeChaningPaintType(paintType);

    mPaintType = paintType;
    mTarget_k->updateDrawGradients();
    mTarget_k->requestGlobalFillStrokeUpdateIfSelected();
}

ColorAnimator *PaintSettings::getColorAnimator() {
    return mColor.data();
}

void PaintSettings::setGradientPoints(GradientPoints* const gradientPoints) {
    mGradientPoints = gradientPoints;
}

StrokeSettings::StrokeSettings(GradientPoints * const grdPts,
                               PathBox * const parent) :
    StrokeSettings(grdPts, parent, QColor(0, 0, 0),
                   PaintType::FLATPAINT, nullptr) {}

StrokeSettings::StrokeSettings(GradientPoints * const grdPts,
                               PathBox * const parent,
                               const QColor &colorT,
                               const PaintType &paintTypeT,
                               Gradient* const gradientT) :
    PaintSettings(grdPts, parent, colorT, paintTypeT, gradientT) {
    prp_setName("stroke");

    ca_addChildAnimator(mLineWidth);
    mLineWidth->prp_setOwnUpdater(SPtrCreate(StrokeWidthUpdater)(parent));
}

void StrokeSettings::showHideChildrenBeforeChaningPaintType(
        const PaintType &newPaintType) {
    PaintSettings::showHideChildrenBeforeChaningPaintType(newPaintType);
    if(getPaintType() == BRUSHPAINT) ca_removeChildAnimator(mBrushSettings);
    if(newPaintType == BRUSHPAINT) ca_addChildAnimator(mBrushSettings);
}

void StrokeSettings::setCurrentStrokeWidth(const qreal &newWidth) {
    mLineWidth->qra_setCurrentValue(newWidth);
}

void StrokeSettings::setCapStyle(const Qt::PenCapStyle &capStyle) {
    mCapStyle = capStyle;
}

void StrokeSettings::setJoinStyle(const Qt::PenJoinStyle &joinStyle) {
    mJoinStyle = joinStyle;
}

void StrokeSettings::setStrokerSettings(QPainterPathStroker * const stroker) {
    stroker->setWidth(mLineWidth->qra_getCurrentValue());
    stroker->setCapStyle(mCapStyle);
    stroker->setJoinStyle(mJoinStyle);
}

void StrokeSettings::setStrokerSettingsSk(SkStroke * const stroker) {
    stroker->setWidth(qrealToSkScalar(mLineWidth->qra_getCurrentValue()));
    stroker->setCap(QCapToSkCap(mCapStyle));
    stroker->setJoin(QJoinToSkJoin(mJoinStyle));
}

void StrokeSettings::setStrokerSettingsForRelFrameSk(const qreal &relFrame,
                                                     SkStroke * const stroker) {
    const qreal widthT = mLineWidth->qra_getEffectiveValueAtRelFrame(relFrame);
    stroker->setWidth(qrealToSkScalar(widthT));
    stroker->setCap(QCapToSkCap(mCapStyle));
    stroker->setJoin(QJoinToSkJoin(mJoinStyle));
}

qreal StrokeSettings::getCurrentStrokeWidth() const {
    return mLineWidth->qra_getCurrentValue();
}

Qt::PenCapStyle StrokeSettings::getCapStyle() const {
    return mCapStyle;
}

Qt::PenJoinStyle StrokeSettings::getJoinStyle() const {
    return mJoinStyle;
}

QrealAnimator *StrokeSettings::getStrokeWidthAnimator() {
    return mLineWidth.data();
}

void StrokeSettings::setOutlineCompositionMode(
        const QPainter::CompositionMode &compositionMode) {
    mOutlineCompositionMode = compositionMode;
}

QPainter::CompositionMode StrokeSettings::getOutlineCompositionMode() {
    return mOutlineCompositionMode;
}

bool StrokeSettings::nonZeroLineWidth() {
    return !isZero4Dec(mLineWidth->qra_getCurrentValue());
}


QrealAnimator *StrokeSettings::getLineWidthAnimator() {
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
        paint->setColor(QColorToSkColor(fPaintColor));
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
        gradColors[i] = QColorToSkColor(col);
        gradPos[i] = currT;

        currX += xInc;
        currY += yInc;
        currT += tInc;
    }
    if(linearGradient) {
        fGradient = SkGradientShader::MakeLinear(gradPoints,
                                                  gradColors,
                                                  gradPos,
                                                  nStops,
                                                  SkShader::kClamp_TileMode);
    } else {
        const QPointF distPt = finalStop - start;
        const SkScalar radius = static_cast<SkScalar>(
                    qSqrt(distPt.x()*distPt.x() + distPt.y()*distPt.y()));
        fGradient = SkGradientShader::MakeRadial(
                        qPointToSk(start), radius,
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
