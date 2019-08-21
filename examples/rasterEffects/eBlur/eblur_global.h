#ifndef EBLUR_GLOBAL_H
#define EBLUR_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(EBLUR_LIBRARY)
#  define EBLUR_EXPORT Q_DECL_EXPORT
#else
#  define EBLUR_EXPORT Q_DECL_IMPORT
#endif

#include "enveCore/RasterEffects/customrastereffect.h"
#include "enveCore/RasterEffects/rastereffectcaller.h"

extern "C" {

EBLUR_EXPORT qsptr<CustomRasterEffect> eCreateNewestVersion();

EBLUR_EXPORT qsptr<CustomRasterEffect> eCreate(const CustomIdentifier &identifier);

EBLUR_EXPORT QString eName();

EBLUR_EXPORT CustomIdentifier eIdentifier();

EBLUR_EXPORT  bool eSupports(const CustomIdentifier &identifier);

}
#endif // EBLUR_GLOBAL_H
