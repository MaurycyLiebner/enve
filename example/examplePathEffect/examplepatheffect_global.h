#ifndef EXAMPLEPATHEFFECT_GLOBAL_H
#define EXAMPLEPATHEFFECT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(EXAMPLEPATHEFFECT_LIBRARY)
#  define EXAMPLEPATHEFFECTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define EXAMPLEPATHEFFECTSHARED_EXPORT Q_DECL_IMPORT
#endif

#include "enveCore/PathEffects/custompatheffect.h"

extern "C" {

EXAMPLEPATHEFFECTSHARED_EXPORT
    qsptr<CustomPathEffect> eCreateNewestVersion();

EXAMPLEPATHEFFECTSHARED_EXPORT
    qsptr<CustomPathEffect> eCreate(
        const CustomIdentifier &identifier);

EXAMPLEPATHEFFECTSHARED_EXPORT
    QString eName();

EXAMPLEPATHEFFECTSHARED_EXPORT
    CustomIdentifier eIdentifier();

EXAMPLEPATHEFFECTSHARED_EXPORT
    bool eSupports(const CustomIdentifier &identifier);

}
#endif // EXAMPLEPATHEFFECT_GLOBAL_H
