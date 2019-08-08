#ifndef EXAMPLERASTEREFFECT_GLOBAL_H
#define EXAMPLERASTEREFFECT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(EXAMPLERASTEREFFECT_LIBRARY)
#  define EXAMPLERASTEREFFECTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define EXAMPLERASTEREFFECTSHARED_EXPORT Q_DECL_IMPORT
#endif

#include "enveCore/RasterEffects/customrastereffect.h"
#include "enveCore/RasterEffects/rastereffectcaller.h"

extern "C" {

EXAMPLERASTEREFFECTSHARED_EXPORT
    qsptr<CustomRasterEffect> eCreateNewestVersion();

EXAMPLERASTEREFFECTSHARED_EXPORT
    qsptr<CustomRasterEffect> eCreate(const CustomIdentifier &identifier);

EXAMPLERASTEREFFECTSHARED_EXPORT
    QString eName();

EXAMPLERASTEREFFECTSHARED_EXPORT
    CustomIdentifier eIdentifier();

EXAMPLERASTEREFFECTSHARED_EXPORT
    bool eSupports(const CustomIdentifier &identifier);

}
#endif // EXAMPLERASTEREFFECT_GLOBAL_H
