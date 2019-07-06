#include "custompatheffectcreator.h"
#include <QLibrary>

QList<CustomPathEffectCreator> CustomPathEffectCreator::sEffectCreators;

void CustomPathEffectCreator::sLoadCustomPathEffect(const QString &libPath) {
    QLibrary lib(libPath);

    const auto creatorNewFunc = reinterpret_cast<CPathEffectCreatorNewFunc>(
                lib.resolve("createNewestVersionEffect"));
    if(!creatorNewFunc) RuntimeThrow("Could not resolve 'createNewestVersionEffect' symbol");

    const auto creatorFunc = reinterpret_cast<CPathEffectCreatorFunc>(
                lib.resolve("createEffect"));
    if(!creatorFunc) RuntimeThrow("Could not resolve 'createEffect' symbol");

    const auto nameFunc = reinterpret_cast<CPathEffectNameFunc>(
                lib.resolve("effectName"));
    if(!nameFunc) RuntimeThrow("Could not resolve 'effectName' symbol");

    const auto identifierFunc = reinterpret_cast<CPathEffectIdentifierFunc>(
                lib.resolve("effectIdentifier"));
    if(!identifierFunc) RuntimeThrow("Could not resolve 'effectIdentifier' symbol");

    const auto supportFunc = reinterpret_cast<CPathEffectSupport>(
                lib.resolve("supports"));
    if(!supportFunc) RuntimeThrow("Could not resolve 'supports' symbol");

    CustomPathEffectCreator creator(creatorNewFunc, creatorFunc, nameFunc,
                                    identifierFunc, supportFunc);
    sEffectCreators.append(creator);
}
