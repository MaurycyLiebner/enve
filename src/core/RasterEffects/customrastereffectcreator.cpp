#include "customrastereffectcreator.h"

#include <QLibrary>

QList<CustomRasterEffectCreator> CustomRasterEffectCreator::sEffectCreators;

void CustomRasterEffectCreator::sLoadCustomRasterEffect(QGL33 * const gl,
                                                  const QString &libGpu) {
    QLibrary lib(libGpu);

    const auto creatorNewFunc = reinterpret_cast<CRasterEffectCreatorNewFunc>(
                lib.resolve("createNewestVersionEffect"));
    if(!creatorNewFunc) RuntimeThrow("Could not resolve 'createNewestVersionEffect' symbol");

    const auto creatorFunc = reinterpret_cast<CRasterEffectCreatorFunc>(
                lib.resolve("createEffect"));
    if(!creatorFunc) RuntimeThrow("Could not resolve 'createEffect' symbol");

    const auto nameFunc = reinterpret_cast<CRasterEffectNameFunc>(
                lib.resolve("effectName"));
    if(!nameFunc) RuntimeThrow("Could not resolve 'effectName' symbol");

    const auto identifierFunc = reinterpret_cast<CRasterEffectIdentifierFunc>(
                lib.resolve("effectIdentifier"));
    if(!identifierFunc) RuntimeThrow("Could not resolve 'effectIdentifier' symbol");

    const auto supportFunc = reinterpret_cast<CRasterEffectSupport>(
                lib.resolve("supports"));
    if(!supportFunc) RuntimeThrow("Could not resolve 'supports' symbol");

    const auto iniFunc = reinterpret_cast<CRasterEffectInitialization>(
                lib.resolve("initialize"));
    if(iniFunc) iniFunc(gl);

    CustomRasterEffectCreator creator(creatorNewFunc, creatorFunc, nameFunc,
                                   identifierFunc, supportFunc);
    sEffectCreators.append(creator);
}
