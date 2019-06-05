#include "sumpatheffect.h"
#include "pathoperations.h"
#include "Boxes/pathbox.h"

SumPathEffect::SumPathEffect(const bool outlinePathEffect) :
    PathEffect("sum effect", SUM_PATH_EFFECT, outlinePathEffect) {
}

void SumPathEffect::apply(const qreal relFrame,
                          const SkPath &src,
                          SkPath * const dst) {
    Q_UNUSED(relFrame);
    QList<SkPath> paths = gBreakApart(src);
    SkOpBuilder builder;
    for(const auto &path : paths) {
        builder.add(path, SkPathOp::kUnion_SkPathOp);
    }
    builder.resolve(dst);
}
