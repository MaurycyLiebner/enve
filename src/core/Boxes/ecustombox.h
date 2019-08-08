#ifndef ECUSTOMBOX_H
#define ECUSTOMBOX_H
#include "boundingbox.h"
#include "../customidentifier.h"

class eCustomBox : public BoundingBox {
public:
    eCustomBox(const QString& name);

    virtual CustomIdentifier getIdentifier() const = 0;
private:
    void writeIdentifier(QIODevice * const dst) const final;
};

#endif // ECUSTOMBOX_H
