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

#include "sculptpathanimator.h"
#include "Animators/transformanimator.h"
#include "Private/document.h"
#include "Properties/emimedata.h"

using SculptPathKey = InterpolationKeyT<SculptPath>;

SculptPathAnimator::SculptPathAnimator() :
    InterOptimalAnimatorT<SculptPath>("sculpt path") {
    prp_enabledDrawingOnCanvas();
}

QMimeData *SculptPathAnimator::SWT_createMimeData() {
    return new eMimeData(QList<SculptPathAnimator*>() << this);
}

void SculptPathAnimator::prp_drawCanvasControls(
        SkCanvas * const canvas, const CanvasMode mode,
        const float invScale, const bool ctrlPressed) {
    if(mode == CanvasMode::sculptPath) {
        const float clampedInvScale = qBound(0.333f, invScale, 1.f);
        const auto current = getCurrentlyEdited();
        const auto& nodes = current->nodes();
        const auto transform = getTransformAnimator()->getCurrentTransform();
        SkPaint outlinePaint;
        outlinePaint.setStyle(SkPaint::kStroke_Style);
        outlinePaint.setColor(SK_ColorWHITE);
        outlinePaint.setStrokeWidth(clampedInvScale);
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        const auto target = Document::sInstance->fSculptTarget;
        std::function<SkColor(const SculptNode&)> nodeColor;
        switch(target) {
        case SculptTarget::position:
            nodeColor = [](const SculptNode& node) {
                const U8CPU red = qBound(0, qRound(0.01*node.t()*255), 255);
                return SkColorSetARGB(255, red, 0, 255 - red);
            };
            break;
        case SculptTarget::pressure:
            nodeColor = [](const SculptNode& node) {
                const U8CPU red = qBound(0, qRound(node.pressure()*255), 255);
                return SkColorSetARGB(255, red, 0, 255 - red);
            };
            break;
        case SculptTarget::width:
            nodeColor = [](const SculptNode& node) {
                const U8CPU red = qBound(0, qRound(0.01*node.width()*255), 255);
                return SkColorSetARGB(255, red, 0, 255 - red);
            };
            break;
        case SculptTarget::spacing:
            nodeColor = [](const SculptNode& node) {
                const U8CPU red = qBound(0, qRound(0.1*node.spacing()*255), 255);
                return SkColorSetARGB(255, red, 0, 255 - red);
            };
            break;
        case SculptTarget::time:
            nodeColor = [](const SculptNode& node) {
                const U8CPU red = qBound(0, qRound(node.time()*255), 255);
                return SkColorSetARGB(255, red, 0, 255 - red);
            };
            break;
        case SculptTarget::color:
            nodeColor = [](const SculptNode& node) {
                return toSkColor(node.color());
            };
            break;
        }
        const float radius = 3*clampedInvScale;
        for(const auto& node : nodes) {
            paint.setColor(nodeColor(*node));
            const QPointF qAbsPos = transform.map(node->pos());
            const SkPoint skAbsPos = toSkPoint(qAbsPos);
            canvas->drawCircle(skAbsPos, radius, paint);
            canvas->drawCircle(skAbsPos, radius, outlinePaint);
        }
    }
    Property::prp_drawCanvasControls(canvas, mode, invScale, ctrlPressed);
}

void SculptPathAnimator::sculpt(const SculptTarget target,
                                const SculptMode mode,
                                const SculptBrush &brush) {
    const auto toSculpt = getCurrentlyEdited();
    toSculpt->sculpt(target, mode, brush);
    changed();
}

void SculptPathAnimator::applyTransform(const QMatrix &transform) {
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto spKey = static_cast<SculptPathKey*>(key);
        spKey->getValue().applyTransform(transform);
    }
    baseValue().applyTransform(transform);
    prp_afterWholeInfluenceRangeChanged();
}
