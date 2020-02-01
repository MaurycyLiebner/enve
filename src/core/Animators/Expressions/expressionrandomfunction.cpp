#include "expressionrandomfunction.h"

ExpressionValue::sptr ExpressionRandomFunction::sCreate(
        const ExpressionValue::sptr &value) {
    return sptr(new ExpressionRandomFunction("rand", value));
}

qreal ExpressionRandomFunction::calculateValue(const qreal relFrame) const {
    const qreal seed = innerValue(relFrame);
    if(isInteger4Dec(seed)) {
        QRandomGenerator rand(static_cast<quint32>(qRound(seed)));
        return rand.generateDouble();
    } else {
        QRandomGenerator fRand(static_cast<quint32>(qFloor(seed)));
        QRandomGenerator cRand(static_cast<quint32>(qCeil(seed)));
        const qreal cRandWeight = seed - qFloor(seed);
        return fRand.generateDouble()*(1 - cRandWeight) +
                cRand.generateDouble()*cRandWeight;
    }
}
