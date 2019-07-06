#ifndef CUSTOMGPUEFFECT_H
#define CUSTOMGPUEFFECT_H
#include "gpueffect.h"
#include "customidentifier.h"

class CustomGpuEffect : public GpuEffect {
protected:
    CustomGpuEffect(const QString& name);
public:
    virtual CustomIdentifier getIdentifier() const = 0;
private:
    void writeIdentifier(QIODevice * const dst) const final;
};

#endif // CUSTOMGPUEFFECT_H
