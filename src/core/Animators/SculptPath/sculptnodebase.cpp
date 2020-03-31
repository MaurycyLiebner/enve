#include "sculptnodebase.h"
#include "sculptbrush.h"
#include "simplemath.h"

SculptNodeBase::SculptNodeBase(const qreal t, const QPointF &pos) :
    mT(t), mPos(pos) {

}

bool SculptNodeBase::replacePosition(const SculptBrush &brush) {
    const qreal influence = brush.influence(mPos);
    if(isZero4Dec(influence)) return false;
    const QPointF targetPos = brush.closestOuterPoint(mPos);
    setPos(mPos*(1 - influence) + targetPos*influence);
    return true;
}

bool SculptNodeBase::dragPosition(const SculptBrush& brush) {
    const qreal influence = brush.influence(mPos);
    if(isZero4Dec(influence)) return false;
    const QPointF displ = brush.displacement();
    setPos(mPos + displ*influence);
    return true;
}
