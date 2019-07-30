#ifndef EEFFECT_H
#define EEFFECT_H
#include "staticcomplexanimator.h"

class eEffect : public StaticComplexAnimator {
public:
    eEffect(const QString &name);

    virtual void writeIdentifier(QIODevice * const dst) const = 0;
    void writeProperty(QIODevice * const dst) const;
    void readProperty(QIODevice * const src);

    void switchVisible();
    void setVisible(const bool visible);
    bool isVisible() const;
private:
    bool mVisible = true;
};

#endif // EEFFECT_H
