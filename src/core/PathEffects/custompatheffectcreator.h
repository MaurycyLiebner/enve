#ifndef CUSTOMPATHEFFECTCREATOR_H
#define CUSTOMPATHEFFECTCREATOR_H
#include <QList>
#include "smartPointers/ememory.h"
#include "typemenu.h"
#include "PathEffects/custompatheffect.h"

typedef qsptr<CustomPathEffect> (*CPathEffectCreatorNewFunc)();
typedef qsptr<CustomPathEffect> (*CPathEffectCreatorFunc)(
        const CustomIdentifier &identifier);
typedef QString (*CPathEffectNameFunc)();
typedef CustomIdentifier (*CPathEffectIdentifierFunc)();
typedef bool (*CPathEffectSupport)(const CustomIdentifier&);

class CustomPathEffectCreator {
    CustomPathEffectCreator(const CPathEffectCreatorNewFunc creatorNew,
                            const CPathEffectCreatorFunc creator,
                            const CPathEffectNameFunc name,
                            const CPathEffectIdentifierFunc identifier,
                            const CPathEffectSupport support) :
        mCreatorNew(creatorNew), mCreator(creator), mName(name),
        mIdentifier(identifier), mSupport(support) {}
public:
    static void sLoadCustomPathEffect(const QString& libPath);

    static qsptr<CustomPathEffect> sCreateForIdentifier(
            const CustomIdentifier &identifier) {
        for(const auto& creator : sEffectCreators) {
            if(!creator.mSupport(identifier)) continue;
            return creator.mCreator(identifier);
        }
        return nullptr;
    }

    template <typename U>
    static void sAddToMenu(PropertyMenu * const menu,
                           const U &adder) {
        for(const auto& creator : sEffectCreators)
            sAddToMenu(menu, creator, adder);
    }
private:
    template <typename U>
    static void sAddToMenu(PropertyMenu * const menu,
                           const CustomPathEffectCreator& creator,
                           const U &adder) {
        menu->addPlainAction<BoundingBox>(creator.mName(),
        [adder, creator](BoundingBox * box) {
            const auto cEffect = creator.mCreatorNew();
            (box->*adder)(qSharedPointerCast<PathEffect>(cEffect));
        });
    }
    static QList<CustomPathEffectCreator> sEffectCreators;

    CPathEffectCreatorNewFunc mCreatorNew;
    CPathEffectCreatorFunc mCreator;
    CPathEffectNameFunc mName;
    CPathEffectIdentifierFunc mIdentifier;
    CPathEffectSupport mSupport;
};

#endif // CUSTOMPATHEFFECTCREATOR_H
