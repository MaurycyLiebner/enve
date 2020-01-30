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
    BoundingBox(eBoxType::internalLink) {
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
        prp_setName(linkTarget->prp_getName() + " link");
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
        prp_setName("empty link");
    }
    planUpdate(UpdateReason::userChange);
}

QPointF InternalLinkBox::getRelCenterPosition() {
    if(!getLinkTarget()) return QPointF();
    return getLinkTarget()->getRelCenterPosition();
}

BoundingBox *InternalLinkBox::getLinkTarget() const {
    return mLinkTarget;
}

bool InternalLinkBox::isParentLink() const {
    if(!mParentGroup) return false;
    return mParentGroup->SWT_isLinkBox();
}

stdsptr<BoxRenderData> InternalLinkBox::createRenderData() {
    if(!getLinkTarget()) return nullptr;
    const auto renderData = getLinkTarget()->createRenderData();
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
    if(!getLinkTarget()) return false;
    return getLinkTarget()->relPointInsidePath(relPos);
}

bool InternalLinkBox::isFrameInDurationRect(const int relFrame) const {
    if(!getLinkTarget()) return false;
    return BoundingBox::isFrameInDurationRect(relFrame) &&
            getLinkTarget()->isFrameInDurationRect(relFrame);
}

bool InternalLinkBox::isFrameFInDurationRect(const qreal relFrame) const {
    if(!getLinkTarget()) return false;
    return BoundingBox::isFrameFInDurationRect(relFrame) &&
            getLinkTarget()->isFrameFInDurationRect(relFrame);
}

HardwareSupport InternalLinkBox::hardwareSupport() const {
    if(!getLinkTarget()) return BoundingBox::hardwareSupport();
    return getLinkTarget()->hardwareSupport();
}

FrameRange InternalLinkBox::prp_getIdenticalRelRange(const int relFrame) const {
    FrameRange range{FrameRange::EMIN, FrameRange::EMAX};
    if(mVisible && getLinkTarget())
        range *= BoundingBox::prp_getIdenticalRelRange(relFrame);
    else return range;
    auto targetRange = getLinkTarget()->prp_getIdenticalRelRange(relFrame);
    return range*targetRange;
}

FrameRange InternalLinkBox::prp_relInfluenceRange() const {
    FrameRange inflRange;
    if(mDurationRectangle) inflRange = mDurationRectangle->getRelFrameRange();
    else inflRange = ComplexAnimator::prp_relInfluenceRange();
    if(getLinkTarget()) {
        return inflRange*getLinkTarget()->prp_relInfluenceRange();
    } else return inflRange;
}

int InternalLinkBox::prp_getRelFrameShift() const {
    if(getLinkTarget()) {
        return getLinkTarget()->prp_getRelFrameShift() +
                BoundingBox::prp_getRelFrameShift();
    } else return BoundingBox::prp_getRelFrameShift();
}

QMatrix InternalLinkBox::getRelativeTransformAtFrame(const qreal relFrame) {
    if(isParentLink()) {
        return getLinkTarget()->getRelativeTransformAtFrame(relFrame);
    } else {
        return BoundingBox::getRelativeTransformAtFrame(relFrame);
    }
}

QMatrix InternalLinkBox::getTotalTransformAtFrame(const qreal relFrame) {
    if(isParentLink()) {
        const auto linkTarget = getLinkTarget();
        return linkTarget->getRelativeTransformAtFrame(relFrame)*
                mParentGroup->getTotalTransformAtFrame(relFrame);
    } else {
        return BoundingBox::getTotalTransformAtFrame(relFrame);
    }
}
