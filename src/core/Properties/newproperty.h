#ifndef NEWPROPERTY_H
#define NEWPROPERTY_H

#include "../ReadWrite/ereadstream.h"
#include "../smartPointers/eobject.h"

template <typename T, int V>
class NewProperty : public T {
    e_OBJECT
    using T::T;
public:
    void prp_readProperty(eReadStream &src) override {
        if(src.evFileVersion() < V) return;
        T::prp_readProperty(src);
    }
};

#endif // NEWPROPERTY_H
