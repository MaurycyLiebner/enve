#include "custompatheffect.h"
#include "basicreadwrite.h"
CustomPathEffect::CustomPathEffect(const QString &name) :
    PathEffect(name, PathEffectType::CUSTOM) {}

void CustomPathEffect::writeIdentifier(QIODevice * const dst) const {
    PathEffect::writeIdentifier(dst);
    const auto identifier = getIdentifier();
    const int size = identifier.size();
    dst->write(rcConstChar(&size), sizeof(int));
    dst->write(identifier);
}

QByteArray CustomPathEffect::sReadIdentifier(QIODevice * const src) {
    int size;
    src->read(rcChar(&size), sizeof(int));
    return src->read(size);
}
