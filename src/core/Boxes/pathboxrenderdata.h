#ifndef PATHBOXRENDERDATA_H
#define PATHBOXRENDERDATA_H
#include "boxrenderdata.h"
#include "Animators/paintsettingsanimator.h"

struct PathBoxRenderData : public BoxRenderData {
    PathBoxRenderData(BoundingBox * const parentBox);

    SkPath fEditPath;
    SkPath fPath;
    SkPath fFillPath;
    SkPath fOutlineBasePath;
    SkPath fOutlinePath;
    SkStroke fStroker;
    UpdatePaintSettings fPaintSettings;
    UpdateStrokeSettings fStrokeSettings;

    void updateRelBoundingRect();
    QPointF getCenterPosition();
protected:
    void drawSk(SkCanvas * const canvas);
};

#endif // PATHBOXRENDERDATA_H
