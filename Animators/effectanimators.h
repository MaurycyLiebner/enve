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
                      const qreal &scale,
                      const bool &highQuality);
    qreal getEffectsMargin() const;

    bool isEffectsAnimator() { return true; }

    void saveToSql(QSqlQuery *query, int boundingBoxSqlId);
    void loadFromSql(int boundingBoxSqlId, BoundingBox *box);
};

#endif // EFFECTANIMATORS_H
