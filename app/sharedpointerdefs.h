#ifndef SHAREDPOINTERDEFS_H
#define SHAREDPOINTERDEFS_H

#include "../core/stdpointer.h"
#include "../core/stdselfref.h"
#include "../core/selfref.h"

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
