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

#include "followpathtransformeffect.h"

#include "Boxes/pathbox.h"
#include "Animators/qrealanimator.h"
#include "Animators/transformanimator.h"

FollowPathTransformEffect::FollowPathTransformEffect() :
    TransformEffect("follow path", TransformEffectType::followPath) {
    mTarget = enve::make_shared<BoxTargetProperty>("path");
    mTarget->setValidator<PathBox>();

    mRotate = enve::make_shared<BoolProperty>("rotate");
    mLengthBased = enve::make_shared<BoolProperty>("length based");
    mComplete = enve::make_shared<QrealAnimator>(0, 0, 1, 0.01, "complete");
    mInfluence = enve::make_shared<QrealAnimator>(1, -1, 1, 0.01, "influence");

    connect(mTarget.get(), &BoxTargetProperty::targetSet,
            this, [this](BoundingBox* const newTarget) {
        auto& conn = mTargetConn.assign(newTarget);
        if(newTarget) {
            const auto parent = getFirstAncestor<BoundingBox>();
            const auto parentTransform = parent->getTransformAnimator();
            conn << connect(newTarget, &Property::prp_absFrameRangeChanged,
                            this, [parentTransform](const FrameRange& range,
                            const bool clip) {
                parentTransform->prp_afterChangedAbsRange(range, clip);
            });
            conn << connect(newTarget,
                            &AdvancedTransformAnimator::prp_currentFrameChanged,
                            this, [parentTransform](const UpdateReason reason) {
                parentTransform->prp_afterChangedCurrent(reason);
            });
        }
    });

    ca_addChild(mTarget);
    ca_addChild(mRotate);
    ca_addChild(mLengthBased);
    ca_addChild(mComplete);
    ca_addChild(mInfluence);
}

void FollowPathTransformEffect::applyEffect(
        const qreal relFrame,
        qreal& pivotX, qreal& pivotY,
        qreal& posX, qreal& posY,
        qreal& rot,
        qreal& scaleX, qreal& scaleY,
        qreal& shearX, qreal& shearY,
        BoundingBox* const parent) {
    Q_UNUSED(pivotX);
    Q_UNUSED(pivotY);
    Q_UNUSED(scaleX);
    Q_UNUSED(scaleY);
    Q_UNUSED(shearX);
    Q_UNUSED(shearY);

    if(!parent) return;
    const auto target = static_cast<PathBox*>(mTarget->getTarget());
    if(!target) return;
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    const qreal targetRelFrame = target->prp_absFrameToRelFrameF(absFrame);
    const auto path = target->getParentCoordinatesPath(targetRelFrame);
    QPainterPath qpath = toQPainterPath(path);

    const qreal infl = mInfluence->getEffectiveValue(relFrame);
    qreal per = mComplete->getEffectiveValue(relFrame);
    const bool rotate = mRotate->getValue();
    const bool lengthBased = mLengthBased->getValue();

    if(lengthBased) {
        const qreal length = qpath.length();
        per = qpath.percentAtLength(per*length);
    }
    const auto p1 = qpath.pointAtPercent(per);

    if(rotate) {
        qreal t2 = per + 0.0001;
        const bool reverse = t2 > 1;
        if(reverse) t2 = 0.9999;
        const auto p2 = qpath.pointAtPercent(t2);

        const QLineF baseLine(QPointF(0., 0.), QPointF(100., 0.));
        QLineF l;
        if(reverse) l = QLineF(p2, p1);
        else l = QLineF(p1, p2);
        qreal trackAngle = l.angleTo(baseLine);
        if(trackAngle > 180) trackAngle -= 360;

        rot += trackAngle*infl;
    }

    posX += p1.x()*infl;
    posY += p1.y()*infl;
}
