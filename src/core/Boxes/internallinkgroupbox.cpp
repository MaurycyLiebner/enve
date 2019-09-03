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

#include "internallinkgroupbox.h"
#include "Animators/transformanimator.h"
#include "Timeline/durationrectangle.h"
#include "layerboxrenderdata.h"

InternalLinkGroupBox::InternalLinkGroupBox(ContainerBox * const linkTarget) :
    ContainerBox(TYPE_INTERNAL_LINK_GROUP) {
    setLinkTarget(linkTarget);

    ca_prependChildAnimator(mTransformAnimator.data(), mBoxTarget);
    connect(mBoxTarget.data(), &BoxTargetProperty::targetSet,
            this, &InternalLinkGroupBox::setTargetSlot);
}

InternalLinkGroupBox::~InternalLinkGroupBox() {
    setLinkTarget(nullptr);
}

void InternalLinkGroupBox::writeBoundingBox(eWriteStream& dst) {
    BoundingBox::writeBoundingBox(dst);
}

void InternalLinkGroupBox::readBoundingBox(eReadStream& src) {
    BoundingBox::readBoundingBox(src);
}

void InternalLinkGroupBox::setupRasterEffectsF(const qreal relFrame,
                                         BoxRenderData * const data) {
    if(isParentLink() && getLinkTarget()) {
        getLinkTarget()->setupRasterEffectsF(relFrame, data);
    } else {
        BoundingBox::setupRasterEffectsF(relFrame, data);
    }
}

SkBlendMode InternalLinkGroupBox::getBlendMode() {
    if(isParentLink() && getLinkTarget()) {
        return getLinkTarget()->getBlendMode();
    }
    return BoundingBox::getBlendMode();
}

void InternalLinkGroupBox::setupRenderData(const qreal relFrame,
                                           BoxRenderData * const data) {
    const auto linkTarget = getLinkTarget();
    if(linkTarget) linkTarget->BoundingBox::setupRenderData(relFrame, data);
    ContainerBox::setupRenderData(relFrame, data);
}

ContainerBox *InternalLinkGroupBox::getFinalTarget() const {
    if(!getLinkTarget()) return nullptr;
    if(getLinkTarget()->SWT_isLinkBox()) {
        return static_cast<InternalLinkGroupBox*>(getLinkTarget())->getFinalTarget();
    }
    return getLinkTarget();
}

bool InternalLinkGroupBox::relPointInsidePath(const QPointF &relPos) const {
    if(!getLinkTarget()) return false;
    return getLinkTarget()->relPointInsidePath(relPos);
}

bool InternalLinkGroupBox::isFrameInDurationRect(const int relFrame) const {
    if(!getLinkTarget()) return false;
    return ContainerBox::isFrameInDurationRect(relFrame) &&
            getLinkTarget()->isFrameInDurationRect(relFrame);
}

bool InternalLinkGroupBox::isFrameFInDurationRect(const qreal relFrame) const {
    if(!getLinkTarget()) return false;
    return ContainerBox::isFrameFInDurationRect(relFrame) &&
            getLinkTarget()->isFrameFInDurationRect(relFrame);
}

FrameRange InternalLinkGroupBox::prp_getIdenticalRelRange(const int relFrame) const {
    FrameRange range{FrameRange::EMIN, FrameRange::EMAX};
    if(mVisible) range *= ContainerBox::prp_getIdenticalRelRange(relFrame);
    else return range;
    auto targetRange = getLinkTarget()->prp_getIdenticalRelRange(relFrame);
    return range*targetRange;
}

FrameRange InternalLinkGroupBox::prp_relInfluenceRange() const {
    FrameRange inflRange;
    if(mDurationRectangle) inflRange = mDurationRectangle->getRelFrameRange();
    else inflRange = ComplexAnimator::prp_relInfluenceRange();
    if(getLinkTarget()) {
        return inflRange*getLinkTarget()->prp_relInfluenceRange();
    } else return inflRange;
}

int InternalLinkGroupBox::prp_getRelFrameShift() const {
    if(getLinkTarget()) {
        return getLinkTarget()->prp_getRelFrameShift() +
                ContainerBox::prp_getRelFrameShift();
    } else return ContainerBox::prp_getRelFrameShift();
}

void InternalLinkGroupBox::setTargetSlot(BoundingBox * const target) {
    setLinkTarget(dynamic_cast<ContainerBox*>(target));
}
#include "Sound/singlesound.h"
#include "Sound/esoundlink.h"

void InternalLinkGroupBox::setLinkTarget(ContainerBox * const linkTarget) {
    disconnect(mBoxTarget.data(), nullptr, this, nullptr);
    if(getLinkTarget()) {
        disconnect(getLinkTarget(), nullptr, this, nullptr);
        getLinkTarget()->removeLinkingBox(this);
        removeAllContained();
    }
    if(linkTarget) {
        prp_setName(linkTarget->prp_getName() + " link");
        connect(linkTarget, &Property::prp_nameChanged,
                this, &Property::prp_setName);
        mBoxTarget->setTarget(linkTarget);
        linkTarget->addLinkingBox(this);
        connect(linkTarget, &BoundingBox::prp_absFrameRangeChanged,
                this, [this, linkTarget](const FrameRange& targetAbs) {
            const auto relRange = linkTarget->prp_absRangeToRelRange(targetAbs);
            prp_afterChangedRelRange(relRange);
        });

        connect(linkTarget->getTransformAnimator(), &Property::prp_absFrameRangeChanged,
                this, [this, linkTarget](const FrameRange& targetAbs) {
            const auto relRange = linkTarget->prp_absRangeToRelRange(targetAbs);
            mTransformAnimator->prp_afterChangedRelRange(relRange);
        });

        const auto &boxesList = linkTarget->getContained();
        for(int i = boxesList.count() - 1; i >= 0; i--) {
            const auto& child = boxesList.at(i);
            if(child->SWT_isBoundingBox()) {
                const auto box = static_cast<BoundingBox*>(child.get());
                const auto newLink = box->createLink();
                addContained(newLink);
            } else /*(child->SWT_isSound())*/ {
                const auto sound = static_cast<SingleSound*>(child.get());
                const auto newLink = sound->createLink();
                addContained(newLink);
            }
        }
    } else {
        prp_setName("empty link");
        mBoxTarget->setTarget(nullptr);
    }
    planUpdate(UpdateReason::userChange);
    connect(mBoxTarget.data(), &BoxTargetProperty::targetSet,
            this, &InternalLinkGroupBox::setTargetSlot);
}

QPointF InternalLinkGroupBox::getRelCenterPosition() {
    if(!getLinkTarget()) return QPointF();
    return getLinkTarget()->getRelCenterPosition();
}

ContainerBox *InternalLinkGroupBox::getLinkTarget() const {
    return static_cast<ContainerBox*>(mBoxTarget->getTarget());
}

stdsptr<BoxRenderData> InternalLinkGroupBox::createRenderData() {
    if(!getLinkTarget()) return nullptr;
    auto renderData = getLinkTarget()->createRenderData();
    renderData->fParentBox = this;
    return renderData;
}
