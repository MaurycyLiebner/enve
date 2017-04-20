#include "boxpathpoint.h"
#include "Boxes/boundingbox.h"

BoxPathPoint::BoxPathPoint(BoundingBox *box) :
    MovablePoint(box, TYPE_PATH_POINT) {

}

void BoxPathPoint::startTransform() {
    MovablePoint::startTransform();
    mSavedAbsPos = mapRelativeToAbsolute(mSavedRelPos);
}

void BoxPathPoint::moveByAbs(QPointF absTranslatione) {
    QPointF absPos = mSavedAbsPos + absTranslatione;
    mParent->setPivotAbsPos(absPos, false, true);
}
