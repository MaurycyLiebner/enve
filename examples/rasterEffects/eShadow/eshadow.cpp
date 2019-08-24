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

#include "eshadow.h"

qsptr<CustomRasterEffect> eCreateNewestVersion() {
    // Use default, most up to date, version
    return enve::make_shared<eShadow>();
}

qsptr<CustomRasterEffect> eCreate(
        const CustomIdentifier &identifier) {
    Q_UNUSED(identifier);
    // Choose version based on identifier
    // if(identifier.fVersion == CustomIdentifier::Version{0, 0, 0})
    return enve::make_shared<eShadow>();
}

// Returned value must be unique, lets enve distinguish effects
QString effectId() {
    return "iz4784jsvg6fw7bc3clb";
}

// Name of your effect used in UI
QString eName() {
    return "eShadow";
}

// here specify your effect's most up to date version
CustomIdentifier::Version effectVersion() {
    return { 0, 0, 0 };
}

CustomIdentifier eIdentifier() {
    return { effectId(), eName(), effectVersion() };
}

bool eSupports(const CustomIdentifier &identifier) {
    if(identifier.fEffectId != effectId()) return false;
    if(identifier.fEffectName != eName()) return false;
    return identifier.fVersion == effectVersion();
}

#include "enveCore/Animators/qrealanimator.h"
eShadow::eShadow() :
    CustomRasterEffect(eName().toLower(), HardwareSupport::gpuPreffered, false) {
    mBlurRadius = enve::make_shared<QrealAnimator>("blur radius");
    mOpacity = enve::make_shared<QrealAnimator>("opacity");
    mColor = enve::make_shared<ColorAnimator>();
    mTranslation = enve::make_shared<QPointFAnimator>("translation");

    mBlurRadius->setValueRange(0, 300);
    mBlurRadius->setCurrentBaseValue(10);
    ca_addChild(mBlurRadius);

    mTranslation->setValuesRange(-1000, 1000);
    mTranslation->setBaseValue(QPointF(0, 0));
    ca_addChild(mTranslation);

    mColor->qra_setCurrentValue(Qt::black);
    ca_addChild(mColor);

    mOpacity->setValueRange(0, 9);
    mOpacity->setCurrentBaseValue(1);
    ca_addChild(mOpacity);

    setPropertyForGUI(mColor.data());
}

stdsptr<RasterEffectCaller>
eShadow::getEffectCaller(const qreal relFrame) const {
    const qreal blur = mBlurRadius->getEffectiveValue(relFrame);
    const QColor color = mColor->getColor(relFrame);
    const QPointF trans = mTranslation->getEffectiveValue(relFrame);
    const qreal opacity = mOpacity->getEffectiveValue(relFrame);

    const int iL = qMax(0, qCeil(blur - trans.x()));
    const int iT = qMax(0, qCeil(blur - trans.y()));
    const int iR = qMax(0, qCeil(blur + trans.x()));
    const int iB = qMax(0, qCeil(blur + trans.y()));

    return enve::make_shared<eShadowCaller>(instanceHwSupport(), blur,
                                            color, trans, opacity,
                                            QMargins(iL, iT, iR, iB));
}

CustomIdentifier eShadow::getIdentifier() const {
    return { effectId(), eName(), { 0, 0, 0 } };
}

void eShadowCaller::setupPaint(SkPaint &paint) const {
    const float sigma = mRadius*0.3333333f;
    const auto filter = SkDropShadowImageFilter::Make(
                mTranslation.x(), mTranslation.y(),
                sigma, sigma, toSkColor(mColor),
                SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode,
                nullptr);
    paint.setImageFilter(filter);
    const float opacityM[20] = {
        1, 0, 0, 0, 0,
        0, 1, 0, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 0, mOpacity, 0};
    paint.setColorFilter(SkColorFilters::Matrix(opacityM));
}

void eShadowCaller::processGpu(QGL33 * const gl,
                               GpuRenderTools &renderTools,
                               GpuRenderData &data) {
    Q_UNUSED(gl);
    Q_UNUSED(data);


    const auto canvas = renderTools.requestTargetCanvas(gl);
    canvas->clear(SK_ColorTRANSPARENT);

    const auto srcTex = renderTools.requestSrcTextureImageWrapper();
    canvas->drawImage(srcTex, 0, 0);

    SkPaint paint;
    setupPaint(paint);
    canvas->drawImage(srcTex, 0, 0, &paint);

    canvas->flush();
}

void eShadowCaller::processCpu(CpuRenderTools &renderTools,
                               const CpuRenderData &data) {
    Q_UNUSED(data);

    SkBitmap tile;
    renderTools.requestBackupBitmap().extractSubset(&tile, data.fTexTile);
    SkCanvas canvas(tile);
    canvas.clear(SK_ColorTRANSPARENT);

    const int radCeil = static_cast<int>(ceil(mRadius));
    const auto& srcBtmp = renderTools.fSrcDst;
    const auto& texTile = data.fTexTile;
    auto srcRect = texTile.makeOutset(radCeil, radCeil);
    if(srcRect.intersect(srcRect, srcBtmp.bounds())) {
        SkBitmap tileSrc;
        srcBtmp.extractSubset(&tileSrc, srcRect);
        const int drawX = srcRect.left() - texTile.left();
        const int drawY = srcRect.top() - texTile.top();
        canvas.drawBitmap(tileSrc, drawX, drawY);

        SkPaint paint;
        setupPaint(paint);
        canvas.drawBitmap(tileSrc, drawX, drawY, &paint);
    }

    renderTools.swap();
}
