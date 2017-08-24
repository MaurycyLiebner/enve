#include "singlepathanimator.h"
#include "Animators/pathanimator.h"
#include "nodepoint.h"
#include "undoredo.h"
#include "edge.h"
#include "Boxes/boundingbox.h"
#include "canvas.h"
#include "skiaincludes.h"
#include "pointhelpers.h"

SinglePathAnimator::SinglePathAnimator() :
    ComplexAnimator() {
    prp_setName("path");
}
