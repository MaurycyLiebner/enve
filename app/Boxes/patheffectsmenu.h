#ifndef PATHEFFECTSMENU_H
#define PATHEFFECTSMENU_H
#include "typemenu.h"
#include "PathEffects/patheffectsinclude.h"
#include "boundingbox.h"

namespace PathEffectsMenu {
    template <typename T, typename U>
    void addPathEffectActionToMenu(const QString& text,
                                   BoxTypeMenu * const menu,
                                   const U& adder, const bool outline);
    template <typename U>
    void addPathEffectsActionToMenu(BoxTypeMenu * const menu,
                                    const U &adder, const bool outline);
    void addPathEffectsToActionMenu(BoxTypeMenu * const menu);
}

#endif // PATHEFFECTSMENU_H
