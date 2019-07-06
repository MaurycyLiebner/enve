#ifndef CUSTOMPATHEFFECT_H
#define CUSTOMPATHEFFECT_H
#include "patheffect.h"

class CustomPathEffect : public PathEffect {
protected:
    CustomPathEffect(const QString& name);
public:
    virtual QByteArray getIdentifier() const = 0;
private:
    void writeIdentifier(QIODevice * const dst) const final;
};

#endif // CUSTOMPATHEFFECT_H
