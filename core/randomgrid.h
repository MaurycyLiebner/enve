#ifndef RANDOMGRID_H
#define RANDOMGRID_H
#include "Animators/staticcomplexanimator.h"
#include "Animators/intanimator.h"

class RandomGrid : public StaticComplexAnimator {
    friend class SelfRef;
protected:
    RandomGrid();
public:
    qreal getBaseSeed(const qreal relFrame) const;
    qreal getGridSize(const qreal relFrame) const;

    qreal getRandomValue(const qreal relFrame, const QPointF& pos) const;

    static qreal sGetRandomValue(const qreal baseSeed, const qreal gridSize,
                                 const QPointF& pos);
    static qreal sGetRandomValue(const qreal min, const qreal max,
                                 const qreal baseSeed, const qreal gridSize,
                                 const QPointF& pos);
private:
    static qreal sGetRandomValue(const qreal baseSeed, const QPoint& gridId);

    qsptr<QrealAnimator> mGridSize;
    qsptr<QrealAnimator> mSeed;
};

#endif // RANDOMGRID_H
