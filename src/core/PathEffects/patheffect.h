#ifndef PATHEFFECT_H
#define PATHEFFECT_H
#include "../Animators/eeffect.h"
#include "../skia/skiaincludes.h"
#include "../Animators/staticcomplexanimator.h"
class BoolProperty;
class PathBox;
class BoundingBox;
enum class PathEffectType : short {
    DISPLACE,
    DASH,
    DUPLICATE,
    SOLIDIFY,
    SUM,
    SUB,
    LINES,
    ZIGZAG,
    SPATIAL_DISPLACE,
    SUBDIVIDE,
    CUSTOM,
    TYPE_COUNT
};
class PathEffect;

class PathEffect : public eEffect {
    Q_OBJECT
protected:
    PathEffect(const QString& name, const PathEffectType type);
public:
    virtual void apply(const qreal relFrame,
                       const SkPath &src,
                       SkPath * const dst) = 0;

    bool SWT_isPathEffect() const;

    QMimeData *SWT_createMimeData() final;

    void writeIdentifier(QIODevice * const dst) const;

    PathEffectType getEffectType();
protected:
    const PathEffectType mPathEffectType;
};

#endif // PATHEFFECT_H
