#ifndef CUSTOMGPUEFFECTCREATOR_H
#define CUSTOMGPUEFFECTCREATOR_H
#include <QList>
#include "smartPointers/sharedpointerdefs.h"
#include "typemenu.h"
#include "GPUEffects/customgpueffect.h"

typedef qsptr<CustomGpuEffect> (*CGpuEffectCreatorNewFunc)();
typedef qsptr<CustomGpuEffect> (*CGpuEffectCreatorFunc)(
        const CustomIdentifier &identifier);
typedef QString (*CGpuEffectNameFunc)();
typedef CustomIdentifier (*CGpuEffectIdentifierFunc)();
typedef bool (*CGpuEffectSupport)(const CustomIdentifier&);
typedef void (*CGpuEffectInitialization)(QGL33 * const gl);

class CustomGpuEffectCreator {
    CustomGpuEffectCreator(const CGpuEffectCreatorNewFunc creatorNew,
                           const CGpuEffectCreatorFunc creator,
                           const CGpuEffectNameFunc name,
                           const CGpuEffectIdentifierFunc identifier,
                           const CGpuEffectSupport support) :
        mCreatorNew(creatorNew), mCreator(creator), mName(name),
        mIdentifier(identifier), mSupport(support) {}
public:
    static void sLoadCustomGpuEffect(QGL33 * const gl,
                                     const QString& libGpu);

    static qsptr<CustomGpuEffect> sCreateForIdentifier(
            const CustomIdentifier &identifier) {
        for(const auto& creator : sEffectCreators) {
            if(!creator.mSupport(identifier)) continue;
            return creator.mCreator(identifier);
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
                           const CustomGpuEffectCreator& creator,
                           const U &adder) {
        menu->addPlainAction<BoundingBox>(creator.mName(),
        [adder, creator](BoundingBox * box) {
            const auto cEffect = creator.mCreatorNew();
            (box->*adder)(GetAsSPtr(cEffect, GpuEffect));
        });
    }
    static QList<CustomGpuEffectCreator> sEffectCreators;

    CGpuEffectCreatorNewFunc mCreatorNew;
    CGpuEffectCreatorFunc mCreator;
    CGpuEffectNameFunc mName;
    CGpuEffectIdentifierFunc mIdentifier;
    CGpuEffectSupport mSupport;
};

#endif // CUSTOMGPUEFFECTCREATOR_H
