#ifndef DABTEST_GLOBAL_H
#define DABTEST_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(DABTEST_LIBRARY)
#  define DABTESTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define DABTESTSHARED_EXPORT Q_DECL_IMPORT
#endif

#include "enveCore/RasterEffects/customrastereffect.h"
#include "enveCore/RasterEffects/rastereffectcaller.h"

extern "C" {

DABTESTSHARED_EXPORT
    qsptr<CustomRasterEffect> eCreateNewestVersion();

DABTESTSHARED_EXPORT
    qsptr<CustomRasterEffect> eCreate(
        const CustomIdentifier &identifier);

DABTESTSHARED_EXPORT
    QString eName();

DABTESTSHARED_EXPORT
    CustomIdentifier eIdentifier();

DABTESTSHARED_EXPORT
    bool eSupports(const CustomIdentifier &identifier);
}
#endif // DABTEST_GLOBAL_H
