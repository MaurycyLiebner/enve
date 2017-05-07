#ifndef EFFECTANIMATORS_H
#define EFFECTANIMATORS_H
#include "complexanimator.h"

namespace fmt_filters { struct image; }

class EffectsSettingsWidget;
class PixmapEffect;

class EffectAnimators : public ComplexAnimator
{
public:
    EffectAnimators();

    void addEffect(PixmapEffect *effect);
    void applyEffects(BoundingBox *target, QImage *imgPtr,
                      const fmt_filters::image &img,
                      const qreal &scale);
    qreal getEffectsMargin() const;

    int prp_saveToSql(QSqlQuery *query, const int &boundingBoxSqlId);
    void prp_loadFromSql(const int &boundingBoxSqlId);
    void setParentBox(BoundingBox *box);
    BoundingBox *getParentBox() {
        return mParentBox;
    }

    bool hasEffects();

    void makeDuplicate(Property *target);
    Property *makeDuplicate() {
        return NULL;
    }

    bool SWT_isEffectAnimators() { return true; }
private:
    BoundingBox *mParentBox;
};

#endif // EFFECTANIMATORS_H
