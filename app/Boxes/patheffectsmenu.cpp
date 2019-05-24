#include "patheffectsmenu.h"

template <typename T, typename U>
void PathEffectsMenu::addPathEffectActionToMenu(
        const QString& text, BoxTypeMenu * const menu,
        const U& adder, const bool& outline) {
    menu->addPlainAction<BoundingBox>(text, [adder, outline](BoundingBox * box) {
        (box->*adder)(SPtrCreateTemplated(T)(outline));
    });
}

template <typename U>
void PathEffectsMenu::addPathEffectsActionToMenu(
        BoxTypeMenu * const menu,
        const U &adder, const bool &outline) {
    addPathEffectActionToMenu<DisplacePathEffect>(
                "Displace", menu, adder, outline);
    addPathEffectActionToMenu<DuplicatePathEffect>(
                "Duplicate", menu, adder, outline);
    addPathEffectActionToMenu<SubPathEffect>(
                "Sub-Path", menu, adder, outline);
    addPathEffectActionToMenu<SolidifyPathEffect>(
                "Solidify", menu, adder, outline);
    addPathEffectActionToMenu<OperationPathEffect>(
                "Operation", menu, adder, outline);
    addPathEffectActionToMenu<SumPathEffect>(
                "Sum", menu, adder, outline);
    addPathEffectActionToMenu<LinesPathEffect>(
                "Lines", menu, adder, outline);
    addPathEffectActionToMenu<ZigZagPathEffect>(
                "ZigZag", menu, adder, outline);
}

void PathEffectsMenu::addPathEffectsToActionMenu(BoxTypeMenu * const menu) {
    const auto pathEffectsMenu = menu->addMenu("Path Effects");
    addPathEffectsActionToMenu(pathEffectsMenu,
                               &BoundingBox::addPathEffect, false);

    const auto fillPathEffectsMenu = menu->addMenu("Fill Effects");
    addPathEffectsActionToMenu(fillPathEffectsMenu,
                               &BoundingBox::addFillPathEffect, false);

    const auto outlinePathEffectsMenu = menu->addMenu("Outline Effects");
    addPathEffectsActionToMenu(outlinePathEffectsMenu,
                               &BoundingBox::addOutlinePathEffect, true);
}
