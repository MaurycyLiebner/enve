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

#include "boxtargetproperty.h"
#include "Animators/complexanimator.h"
#include "Boxes/boundingbox.h"

BoxTargetProperty::BoxTargetProperty(const QString &name) :
    Property(name) {}

BoundingBox* BoxTargetProperty::getTarget() const {
    return mTarget_d;
}

void BoxTargetProperty::setTarget(BoundingBox* const box) {
    mTarget_d = box;

    prp_afterWholeInfluenceRangeChanged();
    emit targetSet(box);
}

bool BoxTargetProperty::SWT_dropSupport(const QMimeData * const data) {
    if(!eMimeData::sHasType<BoundingBox>(data)) return false;
    const auto eData = static_cast<const eMimeData*>(data);
    const auto bData = static_cast<const eDraggedObjects*>(eData);
    return bData->count() == 1;
}

bool BoxTargetProperty::SWT_drop(const QMimeData * const data) {
    const auto eData = static_cast<const eMimeData*>(data);
    const auto bData = static_cast<const eDraggedObjects*>(eData);
    const auto obj = bData->getObject<BoundingBox>(0);
    setTarget(obj);
    return true;
}

void BoxTargetProperty::writeProperty(eWriteStream& dst) const {
    const auto targetBox = mTarget_d.data();
    int targetWriteId = -1;
    int targetDocumentId = -1;

    if(targetBox) {
        if(targetBox->getWriteId() < 0) targetBox->assignWriteId();
        targetWriteId = targetBox->getWriteId();
        targetDocumentId = targetBox->getDocumentId();
    }
    dst << targetWriteId;
    dst << targetDocumentId;
}

void BoxTargetProperty::readProperty(eReadStream& src) {
    int targetReadId;
    src >> targetReadId;
    int targetDocumentId;
    src >> targetDocumentId;
    const auto targetBox = BoundingBox::sGetBoxByReadId(targetReadId);
    if(!targetBox && targetReadId >= 0) {
        QPointer<BoxTargetProperty> thisPtr = this;
        WaitingForBoxLoad::BoxReadFunc readFunc =
        [thisPtr](BoundingBox* box) {
            if(!thisPtr) return;
            thisPtr->setTarget(box);
        };
        WaitingForBoxLoad::BoxNeverReadFunc neverReadFunc =
        [thisPtr, targetDocumentId]() {
            if(!thisPtr) return;
            const auto box = BoundingBox::sGetBoxByDocumentId(targetDocumentId);
            thisPtr->setTarget(box);
        };
        const auto func = WaitingForBoxLoad(targetReadId,
                                            readFunc, neverReadFunc);
        BoundingBox::sAddWaitingForBoxLoad(func);
    } else {
        setTarget(targetBox);
    }
}
