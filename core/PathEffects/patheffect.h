#ifndef PATHEFFECT_H
#define PATHEFFECT_H
#include "Animators/complexanimator.h"
#include "skia/skiaincludes.h"
#include "Animators/staticcomplexanimator.h"
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

typedef PropertyMimeData<PathEffect,
    InternalMimeData::PATH_EFFECT> PathEffectMimeData;

class PathEffect : public StaticComplexAnimator {
protected:
    PathEffect(const QString& name,
               const PathEffectType type);
public:
    virtual void apply(const qreal relFrame,
                       const SkPath &src,
                       SkPath * const dst) = 0;

    bool SWT_isPathEffect() const;
    QMimeData *SWT_createMimeData() final;
    void prp_startDragging() final;

    virtual void writeIdentifier(QIODevice * const dst) const;
    void writeProperty(QIODevice * const dst) const final;
    void readProperty(QIODevice * const src) final;

    PathEffectType getEffectType();

    void switchVisible();
    void setVisible(const bool bT);
    bool isVisible() const;
protected:
    bool mVisible = true;
    const PathEffectType mPathEffectType;
};

#endif // PATHEFFECT_H
