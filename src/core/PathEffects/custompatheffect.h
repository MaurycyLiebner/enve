#ifndef CUSTOMPATHEFFECT_H
#define CUSTOMPATHEFFECT_H
#include "patheffect.h"
#include "../customidentifier.h"

class CustomPathEffect : public PathEffect {
public:
    CustomPathEffect(const QString& name);

    virtual CustomIdentifier getIdentifier() const = 0;
private:
    void writeIdentifier(eWriteStream& dst) const final;
};

#endif // CUSTOMPATHEFFECT_H
