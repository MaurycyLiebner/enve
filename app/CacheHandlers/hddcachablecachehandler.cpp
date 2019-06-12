#include "hddcachablecachehandler.h"

void HDDCachableCacheHandler::blockConts(
        const FrameRange &range, const bool blocked) {
    const auto its = itRange(range);
    for(auto it = its.first; it != its.second; it++) {
        extractElement(it)->setBlocked(blocked);
    }
}

#include "pointhelpers.h"
void HDDCachableCacheHandler::drawCacheOnTimeline(QPainter * const p,
                                                  const QRectF& drawRect,
                                                  const int startFrame,
                                                  const int endFrame,
                                                  const qreal unit) const {
    if(startFrame > endFrame) return;
    p->save();
    const qreal quStartFrame = startFrame/unit;
    const int uStartFrame = isInteger4Dec(quStartFrame) ?
                qRound(quStartFrame): qFloor(quStartFrame);
    const qreal pixelsPerFrame = drawRect.width()/
            (endFrame - startFrame + 1);
//    if(!isOne4Dec(unit))
//        p->translate((uStartFrame - quStartFrame)*pixelsPerFrame, 0);

    p->setBrush(QColor(0, 255, 0, 75));
    p->setPen(Qt::NoPen);

    int lastDrawnFrame = uStartFrame;
    int lastDrawX = qRound(drawRect.x());
    //bool lastStoresInMemory = true;
    for(auto it = itAtOrAfterFrame(uStartFrame); it != end(); it++) {
        const auto cont = extractElement(it);
        const int minFrame = qMax(startFrame - 1,
                                  qRound(cont->getRangeMin()*unit));
        if(minFrame > endFrame + 1) break;
        const int afterMaxFrame = qMin(endFrame + 1,
                                       qRound((cont->getRangeMax() + 1)*unit));
        if(afterMaxFrame < startFrame) continue;

        const int dFrame = minFrame - startFrame;
        int xT = qRound(dFrame*pixelsPerFrame + drawRect.x());

        int widthT = qRound(pixelsPerFrame*(afterMaxFrame - minFrame));
        if(lastDrawnFrame == minFrame) {
            widthT += xT - lastDrawX;
            xT = lastDrawX;
        }
        const bool storesInMemory = cont->storesDataInMemory();
        //if(storesInMemory != lastStoresInMemory) {
            if(storesInMemory) {
                if(cont->blocked()) p->setBrush(QColor(255, 0, 0, 75));
                else p->setBrush(QColor(0, 255, 0, 75));
            } else {
                p->setBrush(QColor(0, 0, 255, 75));
            }
            //lastStoresInMemory = storesInMemory;
        //}

        p->drawRect(xT, qRound(drawRect.top()),
                    widthT, qRound(drawRect.height()));
        lastDrawnFrame = afterMaxFrame;
        lastDrawX = xT + widthT;
    }
    p->restore();
}
