#ifndef ESHADOW_GLOBAL_H
#define ESHADOW_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ESHADOW_LIBRARY)
#  define ESHADOW_EXPORT Q_DECL_EXPORT
#else
#  define ESHADOW_EXPORT Q_DECL_IMPORT
#endif

#include "enveCore/RasterEffects/customrastereffect.h"
#include "enveCore/RasterEffects/rastereffectcaller.h"

extern "C" {

ESHADOW_EXPORT qsptr<CustomRasterEffect> eCreateNewestVersion();

ESHADOW_EXPORT qsptr<CustomRasterEffect> eCreate(const CustomIdentifier &identifier);

ESHADOW_EXPORT QString eName();

ESHADOW_EXPORT CustomIdentifier eIdentifier();

ESHADOW_EXPORT bool eSupports(const CustomIdentifier &identifier);

}
#endif // ESHADOW_GLOBAL_H
