#include "customgpueffectcreator.h"

#include <QLibrary>

QList<CustomGpuEffectCreator> CustomGpuEffectCreator::sEffectCreators;

void CustomGpuEffectCreator::sLoadCustomGpuEffect(QGL33c * const gl,
                                                  const QString &libGpu) {
    QLibrary lib(libGpu);

    const auto creatorNewFunc = reinterpret_cast<CGpuEffectCreatorNewFunc>(
                lib.resolve("createNewestVersionEffect"));
    if(!creatorNewFunc) RuntimeThrow("Could not resolve 'createNewestVersionEffect' symbol");

    const auto creatorFunc = reinterpret_cast<CGpuEffectCreatorFunc>(
                lib.resolve("createEffect"));
    if(!creatorFunc) RuntimeThrow("Could not resolve 'createEffect' symbol");

    const auto nameFunc = reinterpret_cast<CGpuEffectNameFunc>(
                lib.resolve("effectName"));
    if(!nameFunc) RuntimeThrow("Could not resolve 'effectName' symbol");

    const auto identifierFunc = reinterpret_cast<CGpuEffectIdentifierFunc>(
                lib.resolve("effectIdentifier"));
    if(!identifierFunc) RuntimeThrow("Could not resolve 'effectIdentifier' symbol");

    const auto supportFunc = reinterpret_cast<CGpuEffectSupport>(
                lib.resolve("supports"));
    if(!supportFunc) RuntimeThrow("Could not resolve 'supports' symbol");

    const auto iniFunc = reinterpret_cast<CGpuEffectInitialization>(
                lib.resolve("initialize"));
    //if(!iniFunc) RuntimeThrow("Could not resolve 'initialize' symbol");
    if(iniFunc) iniFunc(gl);

    CustomGpuEffectCreator creator(creatorNewFunc, creatorFunc, nameFunc,
                                   identifierFunc, supportFunc);
    sEffectCreators.append(creator);
}
