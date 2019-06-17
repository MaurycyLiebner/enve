#ifndef EFFECTSLOADER_H
#define EFFECTSLOADER_H
#include "offscreenqgl33c.h"

class EffectsLoader : public QObject, protected OffscreenQGL33c {
public:
    EffectsLoader();

    void initialize() {
        OffscreenQGL33c::initialize();
    }
private:
};

#endif // EFFECTSLOADER_H
