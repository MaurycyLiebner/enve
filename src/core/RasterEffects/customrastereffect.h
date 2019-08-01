#ifndef CUSTOMRASTEREFFECT_H
#define CUSTOMRASTEREFFECT_H
#include "rastereffect.h"
#include "../customidentifier.h"

class CustomRasterEffect : public RasterEffect {
protected:
    CustomRasterEffect(const QString& name);
public:
    virtual CustomIdentifier getIdentifier() const = 0;
private:
    void writeIdentifier(QIODevice * const dst) const final;
};

#endif // CUSTOMRASTEREFFECT_H
