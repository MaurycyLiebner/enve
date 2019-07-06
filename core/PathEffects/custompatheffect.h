#ifndef CUSTOMPATHEFFECT_H
#define CUSTOMPATHEFFECT_H
#include "patheffect.h"

class CustomPathEffect : public PathEffect {
protected:
    CustomPathEffect(const QString& name);
public:
    struct Identifier {
        QString fEffectId;
        QString fEffectName;
        QString fVersion;

        void write(QIODevice * const dst) const {
            gWrite(dst, fEffectId);
            gWrite(dst, fEffectName);
            gWrite(dst, fVersion);
        }

        void read(QIODevice * const src) {
            fEffectId = gReadString(src);
            fEffectName = gReadString(src);
            fVersion = gReadString(src);
        }
    };

    virtual CustomPathEffect::Identifier getIdentifier() const = 0;
private:
    void writeIdentifier(QIODevice * const dst) const final;
};

#endif // CUSTOMPATHEFFECT_H
