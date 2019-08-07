#ifndef CUSTOMRasterEffectCREATOR_H
#define CUSTOMRasterEffectCREATOR_H
#include <QList>
#include "smartPointers/ememory.h"
#include "typemenu.h"
#include "RasterEffects/customrastereffect.h"

typedef qsptr<CustomRasterEffect> (*CRasterEffectCreatorNewFunc)();
typedef qsptr<CustomRasterEffect> (*CRasterEffectCreatorFunc)(
        const CustomIdentifier &identifier);
typedef QString (*CRasterEffectNameFunc)();
typedef CustomIdentifier (*CRasterEffectIdentifierFunc)();
typedef bool (*CRasterEffectSupport)(const CustomIdentifier&);
typedef void (*CRasterEffectInitialization)(QGL33 * const gl);

class CustomRasterEffectCreator {
    CustomRasterEffectCreator(const CRasterEffectCreatorNewFunc creatorNew,
                              const CRasterEffectCreatorFunc creator,
                              const CRasterEffectNameFunc name,
                              const CRasterEffectIdentifierFunc identifier,
                              const CRasterEffectSupport support) :
        mCreatorNew(creatorNew), mCreator(creator), mName(name),
        mIdentifier(identifier), mSupport(support) {}
public:
    static void sLoadCustomRasterEffect(QGL33 * const gl,
                                     const QString& libGpu);

    static qsptr<CustomRasterEffect> sCreateForIdentifier(
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
                           const CustomRasterEffectCreator& creator,
                           const U &adder) {
        menu->addPlainAction<BoundingBox>(creator.mName(),
        [adder, creator](BoundingBox * box) {
            const auto cEffect = creator.mCreatorNew();
            (box->*adder)(qSharedPointerCast<RasterEffect>(cEffect));
        });
    }
    static QList<CustomRasterEffectCreator> sEffectCreators;

    CRasterEffectCreatorNewFunc mCreatorNew;
    CRasterEffectCreatorFunc mCreator;
    CRasterEffectNameFunc mName;
    CRasterEffectIdentifierFunc mIdentifier;
    CRasterEffectSupport mSupport;
};

#endif // CUSTOMRasterEffectCREATOR_H
