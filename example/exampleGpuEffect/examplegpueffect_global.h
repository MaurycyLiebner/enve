#ifndef EXAMPLEGPUEFFECT_GLOBAL_H
#define EXAMPLEGPUEFFECT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(EXAMPLEGPUEFFECT_LIBRARY)
#  define EXAMPLEGPUEFFECTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define EXAMPLEGPUEFFECTSHARED_EXPORT Q_DECL_IMPORT
#endif

#include "GPUEffects/customgpueffect.h"
#include "smartPointers/selfref.h"

extern "C" {

EXAMPLEGPUEFFECTSHARED_EXPORT
    qsptr<CustomGpuEffect> createNewestVersionEffect();

EXAMPLEGPUEFFECTSHARED_EXPORT
    qsptr<CustomGpuEffect> createEffect(
        const CustomIdentifier &identifier);

EXAMPLEGPUEFFECTSHARED_EXPORT
    QString effectName();

EXAMPLEGPUEFFECTSHARED_EXPORT
    CustomIdentifier effectIdentifier();

EXAMPLEGPUEFFECTSHARED_EXPORT
    bool supports(const CustomIdentifier &identifier);

}
#endif // EXAMPLEGPUEFFECT_GLOBAL_H
