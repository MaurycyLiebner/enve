#ifndef EEFFECT_H
#define EEFFECT_H
#include "staticcomplexanimator.h"

class eEffect : public StaticComplexAnimator {
public:
    eEffect(const QString &name);

    virtual void writeIdentifier(eWriteStream& dst) const = 0;

    void writeProperty(eWriteStream& dst) const;
    void readProperty(eReadStream& src);

    void switchVisible();
    void setVisible(const bool visible);
    bool isVisible() const;
private:
    bool mVisible = true;
};

#endif // EEFFECT_H
