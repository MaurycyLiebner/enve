#ifndef EXAMPLEPATHEFFECT_H
#define EXAMPLEPATHEFFECT_H

#include "examplepatheffect_global.h"

class ExamplePathEffect : public CustomPathEffect {
public:
    ExamplePathEffect(const bool outlinePathEffect);

    QByteArray getIdentifier() const;

    void read(const QByteArray &identifier, QIODevice * const dst);
    void write(QIODevice * const dst) const;

    void apply(const qreal &relFrame,
               const SkPath &src,
               SkPath * const dst);
};

#endif // EXAMPLEPATHEFFECT_H
