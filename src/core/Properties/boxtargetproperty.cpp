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

void BoxTargetProperty::writeProperty(QIODevice * const target) const {
    const auto targetBox = mTarget_d.data();
    int targetWriteId = -1;
    int targetDocumentId = -1;

    if(targetBox) {
        if(targetBox->getWriteId() < 0) targetBox->assignWriteId();
        targetWriteId = targetBox->getWriteId();
        targetDocumentId = targetBox->getDocumentId();
    }
    target->write(rcConstChar(&targetWriteId), sizeof(int));
    target->write(rcConstChar(&targetDocumentId), sizeof(int));
}

void BoxTargetProperty::readProperty(QIODevice * const src) {
    int targetReadId;
    src->read(rcChar(&targetReadId), sizeof(int));
    int targetDocumentId;
    src->read(rcChar(&targetDocumentId), sizeof(int));
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
