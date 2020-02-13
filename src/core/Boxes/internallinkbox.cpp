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

#include "internallinkbox.h"
#include "GUI/edialogs.h"
#include "canvas.h"
#include "Timeline/durationrectangle.h"
#include "Animators/transformanimator.h"
#include "skia/skiahelpers.h"
#include "videobox.h"
#include "Sound/singlesound.h"

InternalLinkBox::InternalLinkBox(BoundingBox * const linkTarget) :
    BoundingBox("Link", eBoxType::internalLink) {
    setLinkTarget(linkTarget);
    ca_prependChild(mTransformAnimator.data(), mBoxTarget);
    connect(mBoxTarget.data(), &BoxTargetProperty::targetSet,
            this, &InternalLinkBox::setLinkTarget);
}

void InternalLinkBox::setLinkTarget(BoundingBox * const linkTarget) {
    mSound.reset();
    if(mLinkTarget) mLinkTarget->removeLinkingBox(this);
    auto& conn = mLinkTarget.assign(linkTarget);
    mBoxTarget->setTarget(linkTarget);
    if(linkTarget) {
        rename(linkTarget->prp_getName() + " Link 0");
        linkTarget->addLinkingBox(this);
        conn << connect(linkTarget, &BoundingBox::prp_absFrameRangeChanged,
                this, [this, linkTarget](const FrameRange& range) {
            const auto relRange = linkTarget->prp_absRangeToRelRange(range);
            prp_afterChangedRelRange(relRange);
        });
        if(linkTarget->SWT_isVideoBox()) {
            const auto vidBox = static_cast<VideoBox*>(linkTarget);
            mSound = vidBox->sound()->createLink();
            conn << connect(this, &eBoxOrSound::parentChanged,
                            mSound.get(), &eBoxOrSound::setParentGroup);
            mSound->setParentGroup(mParentGroup);
        }
    } else {
        rename("Empty Link 0");
    }
    planUpdate(UpdateReason::userChange);
}

QPointF InternalLinkBox::getRelCenterPosition() {
    const auto linkTarget = getLinkTarget();
    if(!linkTarget) return QPointF();
    return linkTarget->getRelCenterPosition();
}

BoundingBox *InternalLinkBox::getLinkTarget() const {
    return mLinkTarget;
}

bool InternalLinkBox::isParentLink() const {
    if(!mParentGroup) return false;
    return mParentGroup->SWT_isLinkBox();
}

stdsptr<BoxRenderData> InternalLinkBox::createRenderData() {
    const auto linkTarget = getLinkTarget();
    if(!linkTarget) return nullptr;
    const auto renderData = linkTarget->createRenderData();
    renderData->fParentBox = this;
    return renderData;
}

void InternalLinkBox::setupRenderData(const qreal relFrame,
                                      BoxRenderData * const data,
                                      Canvas* const scene) {
    const auto linkTarget = getLinkTarget();
    if(linkTarget) linkTarget->setupRenderData(relFrame, data, scene);
    BoundingBox::setupRenderData(relFrame, data, scene);
}

SkBlendMode InternalLinkBox::getBlendMode() const {
    if(isParentLink()) return getLinkTarget()->getBlendMode();
    return BoundingBox::getBlendMode();
}

bool InternalLinkBox::relPointInsidePath(const QPointF &relPos) const {
    const auto linkTarget = getLinkTarget();
    if(!linkTarget) return false;
    return linkTarget->relPointInsidePath(relPos);
}

bool InternalLinkBox::isFrameInDurationRect(const int relFrame) const {
    const auto linkTarget = getLinkTarget();
    if(!linkTarget) return false;
    return BoundingBox::isFrameInDurationRect(relFrame) &&
           linkTarget->isFrameInDurationRect(relFrame);
}

bool InternalLinkBox::isFrameFInDurationRect(const qreal relFrame) const {
    const auto linkTarget = getLinkTarget();
    if(!linkTarget) return false;
    return BoundingBox::isFrameFInDurationRect(relFrame) &&
           linkTarget->isFrameFInDurationRect(relFrame);
}

HardwareSupport InternalLinkBox::hardwareSupport() const {
    const auto linkTarget = getLinkTarget();
    if(!linkTarget) return BoundingBox::hardwareSupport();
    return linkTarget->hardwareSupport();
}

FrameRange InternalLinkBox::prp_getIdenticalRelRange(const int relFrame) const {
    const auto linkTarget = getLinkTarget();
    FrameRange range{FrameRange::EMIN, FrameRange::EMAX};
    if(mVisible && linkTarget)
        range *= BoundingBox::prp_getIdenticalRelRange(relFrame);
    else return range;
    auto targetRange = linkTarget->prp_getIdenticalRelRange(relFrame);
    return range*targetRange;
}

FrameRange InternalLinkBox::prp_relInfluenceRange() const {
    const auto linkTarget = getLinkTarget();
    FrameRange inflRange;
    if(mDurationRectangle) inflRange = mDurationRectangle->getRelFrameRange();
    else inflRange = ComplexAnimator::prp_relInfluenceRange();
    if(linkTarget) {
        return inflRange*linkTarget->prp_relInfluenceRange();
    } else return inflRange;
}

int InternalLinkBox::prp_getRelFrameShift() const {
    const auto linkTarget = getLinkTarget();
    if(linkTarget) {
        return linkTarget->prp_getRelFrameShift() +
               BoundingBox::prp_getRelFrameShift();
    } else return BoundingBox::prp_getRelFrameShift();
}

QMatrix InternalLinkBox::getRelativeTransformAtFrame(const qreal relFrame) {
    if(isParentLink()) {
        const auto linkTarget = getLinkTarget();
        if(!linkTarget) return QMatrix();
        return linkTarget->getRelativeTransformAtFrame(relFrame);
    } else {
        return BoundingBox::getRelativeTransformAtFrame(relFrame);
    }
}

QMatrix InternalLinkBox::getTotalTransformAtFrame(const qreal relFrame) {
    if(isParentLink()) {
        const auto linkTarget = getLinkTarget();
        if(!linkTarget) return QMatrix();
        return linkTarget->getRelativeTransformAtFrame(relFrame)*
               mParentGroup->getTotalTransformAtFrame(relFrame);
    } else {
        return BoundingBox::getTotalTransformAtFrame(relFrame);
    }
}
