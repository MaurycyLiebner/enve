#ifndef GROUPBOX_H
#define GROUPBOX_H
#include "containerbox.h"
class LayerBox;

class GroupBox : public ContainerBox {
    friend class SelfRef;
protected:
    GroupBox();
public:
    static qsptr<GroupBox> sReplaceLayerBox(const qsptr<LayerBox>& src);
    void addActionsToMenu(BoxTypeMenu * const menu);

    bool SWT_isGroupBox() const { return true; }

    void drawPixmapSk(SkCanvas * const canvas,
                      GrContext * const grContext);

    stdsptr<BoundingBoxRenderData> createRenderData();
    void setupRenderData(const qreal &relFrame,
                         BoundingBoxRenderData * const data);
};



#endif // GROUPBOX_H
