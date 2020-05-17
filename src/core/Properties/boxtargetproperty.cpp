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
#include "simpletask.h"

BoxTargetProperty::BoxTargetProperty(const QString &name) :
    Property(name) {}

BoundingBox* BoxTargetProperty::getTarget() const {
    return mTarget_d;
}

void BoxTargetProperty::setTargetAction(BoundingBox* const box) {
    if(box == mTarget_d) return;
    {
        prp_pushUndoRedoName("Set Box Target");
        UndoRedo ur;
        const qptr<BoundingBox> oldValue = mTarget_d.operator BoundingBox *();
        const qptr<BoundingBox> newValue = box;
        ur.fUndo = [this, oldValue]() {
            setTarget(oldValue);
        };
        ur.fRedo = [this, newValue]() {
            setTarget(newValue);
        };
        prp_addUndoRedo(ur);
    }

    setTarget(box);
}

void BoxTargetProperty::setTarget(BoundingBox* const box) {
    if(box == mTarget_d) return;

    mTarget_d.assign(box);
    if(box) {
        mTarget_d << connect(box, &BoundingBox::prp_ancestorChanged,
                             this, [this]() {
            SimpleTask::sScheduleContexted(this, [this]() {
                if(mTarget_d && mTarget_d->getParentScene()) return;
                setTargetAction(nullptr);
            });
        });
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
    setTargetAction(obj);
    return true;
}

void BoxTargetProperty::prp_writeProperty_impl(eWriteStream& dst) const {
    int targetWriteId = -1;
    int targetDocumentId = -1;

    if(mTarget_d) {
        targetWriteId = mTarget_d->getWriteId();
        targetDocumentId = mTarget_d->getDocumentId();
    }
    dst << targetWriteId;
    dst << targetDocumentId;
}

#include "simpletask.h"
void BoxTargetProperty::prp_readProperty_impl(eReadStream& src) {
    int targetReadId;
    src >> targetReadId;
    int targetDocumentId;
    src >> targetDocumentId;
    if(targetReadId == -1 || targetDocumentId == -1) return;
    SimpleTask::sScheduleContexted(this, [this, targetReadId, targetDocumentId]() {
        BoundingBox* targetBox = nullptr;
        if(targetReadId != -1)
            targetBox = BoundingBox::sGetBoxByReadId(targetReadId);
         if(!targetBox && targetDocumentId != -1)
             targetBox = BoundingBox::sGetBoxByDocumentId(targetDocumentId);
         setTarget(targetBox);
    });
}

QDomElement BoxTargetProperty::prp_writePropertyXEV_impl(const XevExporter& exp) const {
    auto result = exp.createElement("ObjectLink");

    int targetWriteId = -1;
    if(mTarget_d) targetWriteId = mTarget_d->getWriteId();
    result.setAttribute("targetId", targetWriteId);

    return result;
}

void BoxTargetProperty::prp_readPropertyXEV_impl(
        const QDomElement& ele, const XevImporter& imp) {
    Q_UNUSED(imp)
    const int targetId = XmlExportHelpers::stringToInt(ele.attribute("targetId"));
    if(targetId == -1) return;
    SimpleTask::sScheduleContexted(this, [this, targetId]() {
        const auto targetObj = BoundingBox::sGetBoxByReadId(targetId);
        if(targetObj) setTarget(targetObj);
    });
}
