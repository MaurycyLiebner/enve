#include "boxpathpoint.h"
#include "Boxes/boundingbox.h"

BoxPathPoint::BoxPathPoint(BoundingBox *box) :
    MovablePoint(box, TYPE_PATH_POINT) {

}
