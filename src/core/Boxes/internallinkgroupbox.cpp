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
#include "BlendEffects/blendeffectboxshadow.h"

InternalLinkGroupBox::InternalLinkGroupBox(ContainerBox * const linkTarget,
                                           const bool innerLink) :
    InternalLinkBoxBase<ContainerBox>(eBoxType::internalLinkGroup, innerLink) {
    prp_setName("Link 0");

    ca_prependChild(mTransformAnimator.data(), mBoxTarget);
    connect(mBoxTarget.data(), &BoxTargetProperty::targetSet,
            this, [this](BoundingBox* const target) {
        const auto cbTarget = enve_cast<ContainerBox*>(target);
        setLinkTarget(cbTarget);
    });
    mBoxTarget->setTarget(linkTarget);
}

void InternalLinkGroupBox::setupRenderData(const qreal relFrame,
                                           BoxRenderData * const data,
                                           Canvas* const scene) {
    const auto linkTarget = getLinkTarget();
    if(linkTarget) linkTarget->BoundingBox::setupRenderData(relFrame, data, scene);
    ContainerBox::setupRenderData(relFrame, data, scene);
}

#include "Sound/eindependentsound.h"
#include "Sound/esoundlink.h"

void InternalLinkGroupBox::insertInnerLinkFor(
        const int id, eBoxOrSound* const obj) {
    if(const auto box = enve_cast<BoundingBox*>(obj)) {
        const auto newLink = box->createLink(true);
        insertContained(id, newLink);
    } else if(const auto sound = enve_cast<eIndependentSound*>(obj)) {
        const auto newLink = sound->createLink();
        insertContained(id, newLink);
    } else if(const auto bebs = enve_cast<BlendEffectBoxShadow*>(obj)) {
        const auto newLink = bebs->createLink();
        insertContained(id, newLink);
    }
}

void InternalLinkGroupBox::setLinkTarget(ContainerBox * const linkTarget) {
    removeAllContained();
    mBoxTarget->setTarget(linkTarget);
    auto& conn = assignLinkTarget(linkTarget);
    if(linkTarget) {
        conn << connect(linkTarget->getTransformAnimator(), &Property::prp_absFrameRangeChanged,
                this, [this, linkTarget](const FrameRange& targetAbs) {
            const auto relRange = linkTarget->prp_absRangeToRelRange(targetAbs);
            mTransformAnimator->prp_afterChangedRelRange(relRange);
        });

        if(linkTarget->isLayer()) promoteToLayer();
        else demoteToGroup();
        conn << connect(linkTarget, &ContainerBox::switchedGroupLayer,
                        this, [this](const eBoxType type) {
            if(type == eBoxType::group) {
                demoteToGroup();
            } else if(type == eBoxType::layer) {
                promoteToLayer();
            }
        });

        conn << connect(linkTarget, &ContainerBox::insertedObject,
                this, &InternalLinkGroupBox::insertInnerLinkFor);
        conn << connect(linkTarget, &ContainerBox::removedObject,
                        this, &ContainerBox::removeContainedFromList);
        conn << connect(linkTarget, &ContainerBox::movedObject,
                        this, qOverload<int, int>(&ContainerBox::moveContainedInList));

        const auto &boxesList = linkTarget->getContained();
        const int iMax = boxesList.count();
        for(int i = 0; i < iMax; i++) {
            const auto& child = boxesList.at(i);
            insertInnerLinkFor(i, child.get());
        }
    }
    planUpdate(UpdateReason::userChange);
}
