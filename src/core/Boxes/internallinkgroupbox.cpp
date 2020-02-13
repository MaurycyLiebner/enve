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

#include "internallinkgroupbox.h"
#include "Animators/transformanimator.h"
#include "Timeline/durationrectangle.h"
#include "layerboxrenderdata.h"

InternalLinkGroupBox::InternalLinkGroupBox(ContainerBox * const linkTarget,
                                           const bool innerLink) :
    InternalLinkBoxBase<ContainerBox>(eBoxType::internalLinkGroup, innerLink) {
    prp_setName("Link 0");

    ca_prependChild(mTransformAnimator.data(), mBoxTarget);
    connect(mBoxTarget.data(), &BoxTargetProperty::targetSet,
            this, [this](BoundingBox* const target) {
        const auto cbTarget = dynamic_cast<ContainerBox*>(target);
        setLinkTarget(cbTarget);
    });
    mBoxTarget->setTarget(linkTarget);
}

bool InternalLinkGroupBox::SWT_isGroupBox() const {
    const auto finalTarget = getFinalTarget();
    if(!finalTarget) return false;
    return finalTarget->SWT_isGroupBox();
}

void InternalLinkGroupBox::setupRenderData(const qreal relFrame,
                                           BoxRenderData * const data,
                                           Canvas* const scene) {
    const auto linkTarget = getLinkTarget();
    if(linkTarget) linkTarget->BoundingBox::setupRenderData(relFrame, data, scene);
    ContainerBox::setupRenderData(relFrame, data, scene);
}

#include "Sound/singlesound.h"
#include "Sound/esoundlink.h"

void InternalLinkGroupBox::setLinkTarget(ContainerBox * const linkTarget) {
    const auto oldLinkTarget = getLinkTarget();
    if(oldLinkTarget) oldLinkTarget->removeLinkingBox(this);
    removeAllContained();
    mBoxTarget->setTarget(linkTarget);
    auto& conn = assignLinkTarget(linkTarget);
    if(linkTarget) {
        conn << connect(linkTarget->getTransformAnimator(), &Property::prp_absFrameRangeChanged,
                this, [this, linkTarget](const FrameRange& targetAbs) {
            const auto relRange = linkTarget->prp_absRangeToRelRange(targetAbs);
            mTransformAnimator->prp_afterChangedRelRange(relRange);
        });

        const auto &boxesList = linkTarget->getContained();
        for(int i = boxesList.count() - 1; i >= 0; i--) {
            const auto& child = boxesList.at(i);
            if(child->SWT_isBoundingBox()) {
                const auto box = static_cast<BoundingBox*>(child.get());
                const auto newLink = box->createLink(true);
                addContained(newLink);
            } else /*(child->SWT_isSound())*/ {
                const auto sound = static_cast<SingleSound*>(child.get());
                const auto newLink = sound->createLink();
                addContained(newLink);
            }
        }
    }
    planUpdate(UpdateReason::userChange);
}
