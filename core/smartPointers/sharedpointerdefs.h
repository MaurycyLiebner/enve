#ifndef SHAREDPOINTERDEFS_H
#define SHAREDPOINTERDEFS_H

#include "stdpointer.h"
#include "stdselfref.h"
#include "selfref.h"

template <typename Ptr, typename List>
extern inline bool listContainsSharedPtr(Ptr ptr, List list) {
    foreach(Ptr ptrT, list) {
        if(ptrT == ptr) return true;
    }
    return false;
}

#define GetAsPtr(baseObjT, derivedClassT) derivedClassT::getAsPtr<derivedClassT>(baseObjT)
#define GetAsSPtr(baseObjT, derivedClassT) derivedClassT::getAsSPtr<derivedClassT>(baseObjT)

#define SPtrCreate(classT) classT::create<classT>

#endif // SHAREDPOINTERDEFS_H
