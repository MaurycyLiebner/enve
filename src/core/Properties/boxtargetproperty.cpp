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

#include "boxtargetproperty.h"
#include "Animators/complexanimator.h"
#include "Boxes/boundingbox.h"
#include "Properties/emimedata.h"

BoxTargetProperty::BoxTargetProperty(const QString &name) :
    Property(name) {}

BoundingBox* BoxTargetProperty::getTarget() const {
    return mTarget_d;
}

void BoxTargetProperty::setTarget(BoundingBox* const box) {
    if(mTarget_d) {
        disconnect(mTarget_d, &BoundingBox::destroyed,
                   this, nullptr);
    }
    mTarget_d = box;
    if(box) {
        connect(box, &BoundingBox::destroyed,
                this, [this]() { setTarget(nullptr); });
    }
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

void BoxTargetProperty::prp_writeProperty(eWriteStream& dst) const {
    int targetWriteId = -1;
    int targetDocumentId = -1;

    if(mTarget_d) {
        if(mTarget_d->getWriteId() < 0) mTarget_d->assignWriteId();
        targetWriteId = mTarget_d->getWriteId();
        targetDocumentId = mTarget_d->getDocumentId();
    }
    dst << targetWriteId;
    dst << targetDocumentId;
}

#include "simpletask.h"
void BoxTargetProperty::prp_readProperty(eReadStream& src) {
    int targetReadId;
    src >> targetReadId;
    int targetDocumentId;
    src >> targetDocumentId;
    if(targetReadId == -1 || targetDocumentId == -1) return;
    SimpleTask::sSchedule([this, targetReadId, targetDocumentId]() {
        BoundingBox* targetBox = nullptr;
        if(targetReadId != -1)
            targetBox = BoundingBox::sGetBoxByReadId(targetReadId);
         if(!targetBox && targetDocumentId != -1)
             targetBox = BoundingBox::sGetBoxByDocumentId(targetDocumentId);
         setTarget(targetBox);
    });
}
