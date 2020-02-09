#include "blendeffectcollection.h"

#include "Boxes/layerboxrenderdata.h"
#include "typemenu.h"

BlendEffectCollection::BlendEffectCollection() :
    BlendEffectCollectionBase("blend effects") {

}

void BlendEffectCollection::prp_setupTreeViewMenu(PropertyMenu * const menu) {
    if(menu->hasActionsForType<BlendEffectCollection>()) return;
    menu->addedActionsForType<BlendEffectCollection>();
    {
        const PropertyMenu::PlainSelectedOp<BlendEffectCollection> aOp =
        [](BlendEffectCollection * coll) {
            coll->addChild(enve::make_shared<BlendEffect>());
        };
        menu->addPlainAction("New Effect", aOp);
    }
    menu->addSeparator();
    BlendEffectCollectionBase::prp_setupTreeViewMenu(menu);
}

void BlendEffectCollection::blendSetup(
        ChildRenderData &data,
        const int index, const qreal relFrame,
        QList<ChildRenderData> &delayed) const {
    const int iMax = ca_getNumberOfChildren();
    for(int i = 0; i < iMax; i++) {
        const auto effect = getChild(i);
        const int dIndex = effect->zIndex(relFrame);
        if(dIndex == 0) continue;
        const int zIndex = index + (qAbs(dIndex) == 1 ? 2*dIndex : dIndex);
        if(!effect->isPathValid()) {
            ChildRenderData iData(data.fData);
            auto& iClip = iData.fClip;
            iClip.fTargetIndex = zIndex;
            delayed << iData;
            data.fClip.fClipOps.append({SkPath(), SkClipOp::kIntersect, false});
            continue;
        }
        const auto clipPath = effect->clipPath(relFrame);

        data.fClip.fClipOps.append({clipPath, SkClipOp::kDifference, false});

        ChildRenderData iData(data.fData);
        auto& iClip = iData.fClip;
        iClip.fTargetIndex = zIndex;
        iClip.fClipOps.append({clipPath, SkClipOp::kIntersect, false});
        delayed << iData;
    }
}

void BlendEffectCollection::drawBlendSetup(
        BoundingBox* const boxToDraw,
        SkCanvas * const canvas,
        const SkFilterQuality filter, int &drawId,
        QList<std::function<bool(int)>> &delayed) const {
    canvas->save();
    const qreal relFrame = anim_getCurrentRelFrame();
    const int iMax = ca_getNumberOfChildren();
    for(int i = 0; i < iMax; i++) {
        const auto effect = getChild(i);
        const int dIndex = effect->zIndex(relFrame);
        if(dIndex <= 0) continue;
        const int zIndex = drawId + (qAbs(dIndex) == 1 ? 2*dIndex : dIndex);
        if(!effect->isPathValid()) {
            delayed << [boxToDraw, zIndex, canvas, filter](const int drawId) {
                if(drawId < zIndex) return false;
                canvas->save();
                boxToDraw->drawPixmapSk(canvas, filter);
                canvas->restore();
                return true;
            };
            continue;
        }
        const auto clipPath = effect->clipPath(relFrame);

        canvas->clipPath(clipPath, SkClipOp::kDifference, false);

        delayed << [boxToDraw,
                    zIndex, clipPath,
                    canvas, filter](const int drawId) {
            if(drawId < zIndex) return false;
            canvas->save();
            canvas->clipPath(clipPath, SkClipOp::kIntersect, false);
            boxToDraw->drawPixmapSk(canvas, filter);
            canvas->restore();
            return true;
        };
    }
    boxToDraw->drawPixmapSk(canvas, filter);
    canvas->restore();
}

void BlendEffectCollection::prp_writeProperty(eWriteStream &dst) const {
    BlendEffectCollectionBase::prp_writeProperty(dst);
    dst << SWT_isVisible();
}

void BlendEffectCollection::prp_readProperty(eReadStream &src) {
    if(src.evFileVersion() < 12) return;
    BlendEffectCollectionBase::prp_readProperty(src);
    bool visible; src >> visible;
    SWT_setVisible(visible);
}
