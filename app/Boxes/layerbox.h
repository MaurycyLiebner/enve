#ifndef LAYERBOX_H
#define LAYERBOX_H
#include "Boxes/containerbox.h"

class PathBox;
class PathEffectAnimators;
class GroupBox;

class LayerBox : public ContainerBox {
    friend class SelfRef;
protected:
    LayerBox(const BoundingBoxType& type = TYPE_LAYER);
public:
    static qsptr<LayerBox> sReplaceGroupBox(const qsptr<GroupBox> &src);

    void addActionsToMenu(BoxTypeMenu * const menu);
    bool SWT_isLayerBox() const { return true; }
    //MovablePoint *getPointAt(const QPointF &absPos, const CanvasMode &currentMode);


//    QPointF getRelCenterPosition();
    void drawPixmapSk(SkCanvas * const canvas,
                      GrContext* const grContext);

    stdsptr<BoundingBoxRenderData> createRenderData();
    void setupRenderData(const qreal &relFrame,
                         BoundingBoxRenderData * const data);
    qsptr<BoundingBox> createLink();

    bool shouldPaintOnImage() const;
    void schedulerProccessed();
};

#endif // LAYERBOX_H
