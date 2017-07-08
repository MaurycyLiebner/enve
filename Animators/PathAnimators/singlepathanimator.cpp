#include "singlepathanimator.h"
#include "Animators/pathanimator.h"
#include "pathpoint.h"
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

void SinglePathAnimator::drawSelected(SkCanvas *canvas,
                                      const CanvasMode &,
                                      const qreal &invScale,
                                      const SkMatrix &combinedTransform) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStrokeWidth(1.5*invScale);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorBLACK);
    SkPath mappedPath = mSkPath;
    mappedPath.transform(combinedTransform);
    canvas->drawPath(mappedPath, paint);
    paint.setStrokeWidth(0.75*invScale);
    paint.setColor(SK_ColorWHITE);
    canvas->drawPath(mappedPath, paint);
}
