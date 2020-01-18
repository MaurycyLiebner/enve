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

#include "paintsettingsanimator.h"
#include "undoredo.h"
#include "colorhelpers.h"
#include "skia/skqtconversions.h"
#include "skia/skiaincludes.h"
#include "Boxes/pathbox.h"
#include "gradientpoints.h"
#include "gradient.h"
#include "Private/document.h"

PaintSettingsAnimator::PaintSettingsAnimator(const QString &name,
                                             GradientPoints * const grdPts,
                                             PathBox * const parent) :
    ComplexAnimator(name),
    mTarget_k(parent), mGradientPoints(grdPts) {}

void PaintSettingsAnimator::setup(const QColor &color,
                                  const PaintType paintType,
                                  Gradient* const gradient) {
    mColor->setColor(color);
    setPaintType(paintType);
    setGradientVar(gradient);
}

void PaintSettingsAnimator::prp_writeProperty(eWriteStream& dst) const {
    mColor->prp_writeProperty(dst);
    dst.write(&mPaintType, sizeof(PaintType));
    dst.write(&mGradientType, sizeof(GradientType));
    const int gradRWId = mGradient ? mGradient->getReadWriteId() : -1;
    dst << gradRWId;
    const int gradDocId = mGradient ? mGradient->getDocumentId() : -1;
    dst << gradDocId;
    mGradientPoints->prp_writeProperty(dst);
}

void PaintSettingsAnimator::prp_readProperty(eReadStream& src) {
    mColor->prp_readProperty(src);
    PaintType paintType;
    src.read(&paintType, sizeof(PaintType));
    src.read(&mGradientType, sizeof(GradientType));
    int gradRWId; src >> gradRWId;
    int gradDocId; src >> gradDocId;
    Gradient* grad = nullptr;
    if(gradRWId != -1) {
        grad = Document::sInstance->getGradientWithRWId(gradRWId);
    }
    if(gradDocId != -1 && !grad) {
        grad = Document::sInstance->getGradientWithDocumentId(gradDocId);
    }
    setGradientVar(grad);
    setPaintType(paintType);
    mGradientPoints->prp_readProperty(src);
}

void PaintSettingsAnimator::setGradientVar(Gradient* const grad) {
    if(grad == mGradient) return;
    if(mGradient) {
        ca_removeChild(mGradient->ref<Gradient>());
        ca_removeChild(mGradientPoints->ref<GradientPoints>());
    }
    if(grad && !mGradient) {
        if(mTarget_k->getFillSettings() == this)
            mTarget_k->resetFillGradientPointsPos();
        else mTarget_k->resetStrokeGradientPointsPos();
    }
    auto& conn = mGradient.assign(grad);
    if(grad) {
        ca_addChild(grad->ref<Gradient>());
        ca_addChild(mGradientPoints->ref<GradientPoints>());
        conn << connect(grad, &Gradient::prp_currentFrameChanged,
                        this, [this]() { mTarget_k->updateDrawGradients(); });
    }
    mTarget_k->updateDrawGradients();
}

QColor PaintSettingsAnimator::getColor() const {
    return mColor->getColor();
}

QColor PaintSettingsAnimator::getColor(const qreal relFrame) const {
    return mColor->getColor(relFrame);
}

void PaintSettingsAnimator::setGradientType(const GradientType type) {
    if(mGradientType == type) return;
    {
        UndoRedo ur;
        const auto oldValue = mGradientType;
        const auto newValue = type;
        ur.fUndo = [this, oldValue]() {
            setGradientType(oldValue);
        };
        ur.fRedo = [this, newValue]() {
            setGradientType(newValue);
        };
        prp_addUndoRedo(ur);
    }
    mGradientType = type;
    prp_afterWholeInfluenceRangeChanged();
}

PaintType PaintSettingsAnimator::getPaintType() const {
    return mPaintType;
}

Gradient *PaintSettingsAnimator::getGradient() const {
    return *mGradient;
}

void PaintSettingsAnimator::setGradient(Gradient* gradient) {
    if(gradient == mGradient) return;
    {
        UndoRedo ur;
        const qptr<Gradient> oldValue = *mGradient;
        const qptr<Gradient> newValue = gradient;
        ur.fUndo = [this, oldValue]() {
            setGradient(oldValue);
        };
        ur.fRedo = [this, newValue]() {
            setGradient(newValue);
        };
        prp_addUndoRedo(ur);
    }
    setGradientVar(gradient);
    mTarget_k->requestGlobalFillStrokeUpdateIfSelected();
}

void PaintSettingsAnimator::setCurrentColor(const QColor &color) {
    mColor->setColor(color);
}

void PaintSettingsAnimator::showHideChildrenBeforeChaningPaintType(
        const PaintType newPaintType) {
    if(mPaintType == FLATPAINT || mPaintType == BRUSHPAINT)
        ca_removeChild(mColor);
    if(newPaintType == FLATPAINT || newPaintType == BRUSHPAINT)
        ca_addChild(mColor);
}

void PaintSettingsAnimator::setPaintType(const PaintType paintType) {
    if(paintType == mPaintType) return;
    {
        UndoRedo ur;
        const auto oldValue = mPaintType;
        const auto newValue = paintType;
        ur.fUndo = [this, oldValue]() {
            setPaintType(oldValue);
        };
        ur.fRedo = [this, newValue]() {
            setPaintType(newValue);
        };
        prp_addUndoRedo(ur);
    }

    showHideChildrenBeforeChaningPaintType(paintType);

    mPaintType = paintType;
    mTarget_k->updateDrawGradients();
    mTarget_k->requestGlobalFillStrokeUpdateIfSelected();
    prp_afterWholeInfluenceRangeChanged();

    SWT_setDisabled(paintType == PaintType::NOPAINT);
}

ColorAnimator *PaintSettingsAnimator::getColorAnimator() {
    return mColor.data();
}

void PaintSettingsAnimator::setGradientPointsPos(const QPointF &pt1,
                                                 const QPointF &pt2) {
    if(!mGradientPoints) return;
    mGradientPoints->setPositions(pt1, pt2);
}

UpdatePaintSettings::UpdatePaintSettings(const QColor &paintColor,
                                         const PaintType paintType) {
    fPaintColor = paintColor;
    fPaintType = paintType;
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
                                         const GradientType gradientType) {
    const int nStops = stops.count();
    QVector<SkPoint> gradPoints(nStops);
    QVector<SkColor> gradColors(nStops);
    QVector<float> gradPos(nStops);

    const float xInc = static_cast<float>(finalStop.x() - start.x());
    const float yInc = static_cast<float>(finalStop.y() - start.y());
    float currX = static_cast<float>(start.x());
    float currY = static_cast<float>(start.y());
    float currT = 0;
    const float tInc = 1.f/(nStops - 1);

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
    if(gradientType == GradientType::LINEAR) {
        fGradient = SkGradientShader::MakeLinear(gradPoints.data(),
                                                 gradColors.data(),
                                                 gradPos.data(), nStops,
                                                 SkTileMode::kClamp);
    } else {
        const QPointF distPt = finalStop - start;
        const qreal radius = qSqrt(pow2(distPt.x()) + pow2(distPt.y()));
        fGradient = SkGradientShader::MakeRadial(
                        toSkPoint(start), toSkScalar(radius),
                        gradColors.data(), gradPos.data(),
                        nStops, SkTileMode::kClamp);
    }
}

UpdateStrokeSettings::UpdateStrokeSettings(const qreal width,
                   const QColor &paintColorT,
                   const PaintType paintTypeT,
                   const QPainter::CompositionMode &outlineCompositionModeT) :
    UpdatePaintSettings(paintColorT, paintTypeT), fOutlineWidth(width) {
    fOutlineCompositionMode = outlineCompositionModeT;
}

UpdateStrokeSettings::UpdateStrokeSettings() {}

void UpdateStrokeSettings::applyPainterSettingsSk(SkPaint *paint) {
    UpdatePaintSettings::applyPainterSettingsSk(paint);
}
