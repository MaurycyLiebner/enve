#include "patheffectsmenu.h"

template <typename T, typename U>
void PathEffectsMenu::addPathEffectActionToMenu(
        const QString& text, BoxTypeMenu * const menu,
        const U& adder) {
    menu->addPlainAction<BoundingBox>(text, [adder](BoundingBox * box) {
        (box->*adder)(SPtrCreateTemplated(T)());
    });
}

#include "PathEffects/custompatheffectcreator.h"
template <typename U>
void PathEffectsMenu::addPathEffectsActionToMenu(
        BoxTypeMenu * const menu, const U &adder) {
    addPathEffectActionToMenu<DisplacePathEffect>(
                "Displace", menu, adder);
    addPathEffectActionToMenu<SpatialDisplacePathEffect>(
                "Spatial Displace", menu, adder);
    addPathEffectActionToMenu<DashPathEffect>(
                "Dash", menu, adder);
    addPathEffectActionToMenu<DuplicatePathEffect>(
                "Duplicate", menu, adder);
    addPathEffectActionToMenu<SubPathEffect>(
                "Sub-Path", menu, adder);
    addPathEffectActionToMenu<SolidifyPathEffect>(
                "Solidify", menu, adder);
    addPathEffectActionToMenu<SumPathEffect>(
                "Sum", menu, adder);
    addPathEffectActionToMenu<LinesPathEffect>(
                "Lines", menu, adder);
    addPathEffectActionToMenu<ZigZagPathEffect>(
                "ZigZag", menu, adder);
    addPathEffectActionToMenu<SubdividePathEffect>(
                "Subdivide", menu, adder);
    CustomPathEffectCreator::sAddToMenu(menu, adder);
}

void PathEffectsMenu::addPathEffectsToActionMenu(BoxTypeMenu * const menu) {
    const auto pathEffectsMenu = menu->addMenu("Path Effects");
    addPathEffectsActionToMenu(pathEffectsMenu,
                               &BoundingBox::addPathEffect);

    const auto fillPathEffectsMenu = menu->addMenu("Fill Effects");
    addPathEffectsActionToMenu(fillPathEffectsMenu,
                               &BoundingBox::addFillPathEffect);

    const auto outlineBasePathEffectsMenu = menu->addMenu("Outline Base Effects");
    addPathEffectsActionToMenu(outlineBasePathEffectsMenu,
                               &BoundingBox::addOutlineBasePathEffect);

    const auto outlinePathEffectsMenu = menu->addMenu("Outline Effects");
    addPathEffectsActionToMenu(outlinePathEffectsMenu,
                               &BoundingBox::addOutlinePathEffect);
}
