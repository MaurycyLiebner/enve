#ifndef EXAMPLEPATHEFFECT_GLOBAL_H
#define EXAMPLEPATHEFFECT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(EXAMPLEPATHEFFECT_LIBRARY)
#  define EXAMPLEPATHEFFECTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define EXAMPLEPATHEFFECTSHARED_EXPORT Q_DECL_IMPORT
#endif

#include "PathEffects/custompatheffect.h"
#include "smartPointers/selfref.h"

extern "C" {
EXAMPLEPATHEFFECTSHARED_EXPORT
    qsptr<CustomPathEffect> createEffect();

EXAMPLEPATHEFFECTSHARED_EXPORT
    QString effectName();

EXAMPLEPATHEFFECTSHARED_EXPORT
    QByteArray effectIdentifier();

EXAMPLEPATHEFFECTSHARED_EXPORT
    bool supports(const QByteArray &identifier);
}
#endif // EXAMPLEPATHEFFECT_GLOBAL_H
