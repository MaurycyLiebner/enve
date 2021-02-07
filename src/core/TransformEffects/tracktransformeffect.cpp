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

#include "tracktransformeffect.h"

#include "Boxes/boundingbox.h"
#include "Animators/qrealanimator.h"
#include "Animators/transformanimator.h"

TrackTransformEffect::TrackTransformEffect() :
    TransformEffect("track", TransformEffectType::track) {
    mInfluence = enve::make_shared<QrealAnimator>(1, -1, 1, 0.01, "influence");
    mTarget = enve::make_shared<BoxTargetProperty>("target");

    connect(mTarget.get(), &BoxTargetProperty::targetSet,
            this, [this](BoundingBox* const newTarget) {
        auto& conn = mTargetConn.assign(newTarget);
        if(newTarget) {
            const auto parent = getFirstAncestor<BoundingBox>();
            const auto parentTransform = parent->getTransformAnimator();
            const auto targetTransform = newTarget->getTransformAnimator();
            conn << connect(targetTransform, &Property::prp_absFrameRangeChanged,
                            this, [parentTransform](const FrameRange& range,
                            const bool clip) {
                parentTransform->prp_afterChangedAbsRange(range, clip);
            });
            conn << connect(targetTransform,
                            &AdvancedTransformAnimator::totalTransformChanged,
                            this, [parentTransform](const UpdateReason reason) {
                parentTransform->prp_afterChangedCurrent(reason);
            });
            conn << connect(parentTransform,
                            &AdvancedTransformAnimator::inheritedTransformChanged,
                            this, [parentTransform](const UpdateReason reason) {
                parentTransform->prp_afterChangedCurrent(reason);
            });
        }
    });

    ca_addChild(mInfluence);
    ca_addChild(mTarget);
}

void TrackTransformEffect::applyEffect(
        const qreal relFrame,
        qreal& pivotX, qreal& pivotY,
        qreal& posX, qreal& posY,
        qreal& rot,
        qreal& scaleX, qreal& scaleY,
        qreal& shearX, qreal& shearY,
        BoundingBox* const parent) {
    Q_UNUSED(pivotX);
    Q_UNUSED(pivotY);
    Q_UNUSED(posX);
    Q_UNUSED(posY);
    Q_UNUSED(scaleX);
    Q_UNUSED(scaleY);
    Q_UNUSED(shearX);
    Q_UNUSED(shearY);

    if(!parent) return;
    const auto target = mTarget->getTarget();
    if(!target) return;
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    const qreal targetRelFrame = target->prp_absFrameToRelFrameF(absFrame);
    const auto targetPos = target->getPivotAbsPos(targetRelFrame);
    const auto parentPos = parent->getPivotAbsPos(relFrame);

    const QLineF baseLine(parentPos, parentPos + QPointF(100., 0.));
    const QLineF trackLine(parentPos, targetPos);
    qreal trackAngle = trackLine.angleTo(baseLine);
    if(trackAngle > 180) trackAngle -= 360;

    const qreal infl = mInfluence->getEffectiveValue(relFrame);

    rot += trackAngle*infl;
}
