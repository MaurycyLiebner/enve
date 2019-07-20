#ifndef SHAREDPOINTERDEFS_H
#define SHAREDPOINTERDEFS_H

#include "stdpointer.h"
#include "stdselfref.h"
#include "selfref.h"

#define GetAsPtr(baseObjT, derivedClassT) \
    derivedClassT::getAsPtr<derivedClassT>(baseObjT)
#define GetAsSPtr(baseObjT, derivedClassT) \
    derivedClassT::getAsSPtr<derivedClassT>(baseObjT)
#define GetAsPtrTemplated(baseObjT, derivedClassT) \
    derivedClassT::template getAsPtr<derivedClassT>(baseObjT)
#define GetAsSPtrTemplated(baseObjT, derivedClassT) \
    derivedClassT::template getAsSPtr<derivedClassT>(baseObjT)

#define SPtrCreate(classT) classT::createSPtr<classT>
#define SPtrCreateTemplated(classT) classT::template createSPtr<classT>

#endif // SHAREDPOINTERDEFS_H
