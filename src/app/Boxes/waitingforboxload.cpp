#include "waitingforboxload.h"
#include "Boxes/boundingbox.h"

WaitingForBoxLoad::WaitingForBoxLoad(const int boxReadId,
                                     const BoxReadFunc &boxRead,
                                     const BoxNeverReadFunc &boxNeverRead) :
    mBoxReadId(boxReadId),
    mBoxReadFunction(boxRead),
    mBoxNeverReadFunction(boxNeverRead) {}

bool WaitingForBoxLoad::boxRead(BoundingBox * const box) const {
    if(box->getReadId() != mBoxReadId) return false;
    if(mBoxReadFunction) mBoxReadFunction(box);
    return true;
}

void WaitingForBoxLoad::boxNeverRead() const {
    if(mBoxNeverReadFunction) mBoxNeverReadFunction();
}
