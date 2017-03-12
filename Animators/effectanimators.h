#ifndef EFFECTANIMATORS_H
#define EFFECTANIMATORS_H
#include "complexanimator.h"

namespace fmt_filters { struct image; }

class EffectsSettingsWidget;

class EffectAnimators : public ComplexAnimator
{
public:
    EffectAnimators();

    void applyEffects(BoundingBox *target, QImage *imgPtr,
                      const fmt_filters::image &img,
                      const qreal &scale);
    qreal getEffectsMargin() const;

    void saveToSql(QSqlQuery *query, int boundingBoxSqlId);
    void loadFromSql(int boundingBoxSqlId, BoundingBox *box);
    void setParentBox(BoundingBox *box);
    BoundingBox *getParentBox() {
        return mParentBox;
    }

    bool hasEffects();
private:
    BoundingBox *mParentBox;
};

#endif // EFFECTANIMATORS_H
