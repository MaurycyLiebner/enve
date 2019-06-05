#include "randomgrid.h"

RandomGrid::RandomGrid() : ComplexAnimator("randomness grid") {
    mSeed = SPtrCreate(QrealAnimator)(qrand() % 9999, 0, 9999, 1, "smooth seed");
    mGridSize = SPtrCreate(QrealAnimator)(100, 1, 9999, 1, "grid size");

    ca_addChildAnimator(mSeed);
    ca_addChildAnimator(mGridSize);
}

qreal RandomGrid::getBaseSeed(const qreal relFrame) const {
    return mSeed->getEffectiveValue(relFrame);
}

qreal RandomGrid::getGridSize(const qreal relFrame) const {
    return mGridSize->getEffectiveValue(relFrame);
}

qreal RandomGrid::getRandomValue(const qreal relFrame, const QPointF &pos) const {
    return sGetRandomValue(getBaseSeed(relFrame), getGridSize(relFrame), pos);
}

void RandomGrid::writeProperty(QIODevice * const target) const {
    mSeed->writeProperty(target);
    mGridSize->writeProperty(target);
}

void RandomGrid::readProperty(QIODevice *target) {
    mSeed->readProperty(target);
    mGridSize->readProperty(target);
}

qreal RandomGrid::sGetRandomValue(const qreal baseSeed, const qreal gridSize,
                                  const QPointF &pos) {
    const QPointF gridIdF = pos/gridSize;
    if(isInteger4Dec(gridIdF.x()) && isInteger4Dec(gridIdF.y())) {
        const QPoint gridId = QPoint{qRound(gridIdF.x()), qRound(gridIdF.y())};
        return sGetRandomValue(baseSeed, gridId);
    }

    const QPoint gridId1{qCeil(gridIdF.x()), qCeil(gridIdF.y())};
    const QPoint gridId2{qFloor(gridIdF.x()), qCeil(gridIdF.y())};
    const QPoint gridId3{qCeil(gridIdF.x()), qFloor(gridIdF.y())};
    const QPoint gridId4{qFloor(gridIdF.x()), qFloor(gridIdF.y())};

    const qreal gridId1Dist = pointToLen(QPointF(gridId1) * gridSize - pos);
    const qreal gridId2Dist = pointToLen(QPointF(gridId2) * gridSize - pos);
    const qreal gridId3Dist = pointToLen(QPointF(gridId3) * gridSize - pos);
    const qreal gridId4Dist = pointToLen(QPointF(gridId4) * gridSize - pos);

    const qreal grid1W = 1 - clamp(gridId1Dist/gridSize, 0, 1);
    const qreal grid2W = 1 - clamp(gridId2Dist/gridSize, 0, 1);
    const qreal grid3W = 1 - clamp(gridId3Dist/gridSize, 0, 1);
    const qreal grid4W = 1 - clamp(gridId4Dist/gridSize, 0, 1);

    const qreal wSum = grid1W + grid2W + grid3W + grid4W;

    const qreal grid1V = sGetRandomValue(baseSeed, gridId1);
    const qreal grid2V = sGetRandomValue(baseSeed, gridId2);
    const qreal grid3V = sGetRandomValue(baseSeed, gridId3);
    const qreal grid4V = sGetRandomValue(baseSeed, gridId4);

    return (grid1V*grid1W + grid2V*grid2W + grid3V*grid3W + grid4V*grid4W)/wSum;
}

qreal RandomGrid::sGetRandomValue(const qreal min, const qreal max,
                                  const qreal baseSeed, const qreal gridSize,
                                  const QPointF &pos) {
    return sGetRandomValue(baseSeed, gridSize, pos)*(max - min) + min;
}

qreal RandomGrid::sGetRandomValue(const qreal baseSeed, const QPoint &gridId) {
    const int seedInc = gridId.x()*100 + gridId.y()*1000;
    if(isInteger4Dec(baseSeed)) {
        QRandomGenerator rand(static_cast<quint32>(qRound(baseSeed) + seedInc));
        return rand.generateDouble();
    } else {
        QRandomGenerator fRand(static_cast<quint32>(qFloor(baseSeed) + seedInc));
        QRandomGenerator cRand(static_cast<quint32>(qCeil(baseSeed) + seedInc));
        const qreal cRandWeight = baseSeed - qFloor(baseSeed);
        return fRand.generateDouble()*(1 - cRandWeight) +
                cRand.generateDouble()*cRandWeight;
    }
}
