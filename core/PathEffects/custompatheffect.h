#ifndef CUSTOMPATHEFFECT_H
#define CUSTOMPATHEFFECT_H
#include "patheffect.h"

class CustomPathEffect : public PathEffect {
protected:
    CustomPathEffect(const QString& name);
public:
    virtual QByteArray getIdentifier() const = 0;

    static QByteArray sReadIdentifier(QIODevice * const src);
    void writeIdentifier(QIODevice * const dst) const;
};

#endif // CUSTOMPATHEFFECT_H
