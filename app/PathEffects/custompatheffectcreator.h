#ifndef CUSTOMPATHEFFECTCREATOR_H
#define CUSTOMPATHEFFECTCREATOR_H
#include <QList>
#include "smartPointers/sharedpointerdefs.h"
#include "typemenu.h"
#include "PathEffects/custompatheffect.h"
class CustomPathEffect;

typedef qsptr<CustomPathEffect> (*CPathEffectCreatorFunc)();
typedef QString (*CPathEffectNameFunc)();
typedef QByteArray (*CPathEffectIdentifierFunc)();
typedef bool (*CPathEffectSupport)(const QByteArray&);

class CustomPathEffectCreator {
    CustomPathEffectCreator(const CPathEffectCreatorFunc creator,
                            const CPathEffectNameFunc name,
                            const CPathEffectIdentifierFunc identifier,
                            const CPathEffectSupport support) :
        mCreator(creator), mName(name),
        mIdentifier(identifier), mSupport(support) {}
public:
    static void sLoadCustomPathEffect(const QString& libPath);

    static qsptr<CustomPathEffect> sCreateForIdentifier(
            const QByteArray &identifier) {
        for(const auto& creator : sEffectCreators) {
            if(!creator.mSupport(identifier)) continue;
            return creator.mCreator();
        }
        return nullptr;
    }

    template <typename U>
    static void sAddToMenu(BoxTypeMenu * const menu,
                           const U &adder) {
        for(const auto& creator : sEffectCreators)
            sAddToMenu(menu, creator, adder);
    }
private:
    template <typename U>
    static void sAddToMenu(BoxTypeMenu * const menu,
                           const CustomPathEffectCreator& creator,
                           const U &adder) {
        menu->addPlainAction<BoundingBox>(creator.mName(),
        [adder, creator](BoundingBox * box) {
            const auto cEffect = creator.mCreator();
            (box->*adder)(GetAsSPtr(cEffect, PathEffect));
        });
    }
    static QList<CustomPathEffectCreator> sEffectCreators;

    CPathEffectCreatorFunc mCreator;
    CPathEffectNameFunc mName;
    CPathEffectIdentifierFunc mIdentifier;
    CPathEffectSupport mSupport;
};

#endif // CUSTOMPATHEFFECTCREATOR_H
