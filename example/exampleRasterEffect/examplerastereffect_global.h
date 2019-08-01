#ifndef EXAMPLERASTEREFFECT_GLOBAL_H
#define EXAMPLERASTEREFFECT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(EXAMPLERasterEffect_LIBRARY)
#  define EXAMPLERASTEREFFECTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define EXAMPLERASTEREFFECTSHARED_EXPORT Q_DECL_IMPORT
#endif

#include "enveCore/RasterEffects/customrastereffect.h"
#include "enveCore/RasterEffects/rastereffectcaller.h"

extern "C" {

EXAMPLERASTEREFFECTSHARED_EXPORT
    qsptr<CustomRasterEffect> createNewestVersionEffect();

EXAMPLERASTEREFFECTSHARED_EXPORT
    qsptr<CustomRasterEffect> createEffect(
        const CustomIdentifier &identifier);

EXAMPLERASTEREFFECTSHARED_EXPORT
    QString effectName();

EXAMPLERASTEREFFECTSHARED_EXPORT
    CustomIdentifier effectIdentifier();

EXAMPLERASTEREFFECTSHARED_EXPORT
    bool supports(const CustomIdentifier &identifier);

}
#endif // EXAMPLERASTEREFFECT_GLOBAL_H
