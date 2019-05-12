#include "hddcachablecachehandler.h"

void HDDCachableCacheHandler::removeRenderContainer(
        const stdsptr<Cont> &cont) {
    mRenderContainers.removeOne(cont);
}

int HDDCachableCacheHandler::insertIdForRelFrame(
        const int &relFrame) const {
    int minId = 0;
    int maxId = mRenderContainers.count();

    while(minId < maxId) {
        const int guess = (minId + maxId)/2;
        const auto& cont = mRenderContainers.at(guess);
        const int contFrame = cont->getRangeMin();
        if(contFrame == relFrame)
            return guess;
        if(contFrame > relFrame) {
            if(guess == maxId) return minId;
            maxId = guess;
        } else if(contFrame < relFrame) {
            if(guess == minId) return maxId;
            minId = guess;
        }
    }
    return 0;
}

bool HDDCachableCacheHandler::idAtRelFrame(const int &relFrame, int *id) const {
    int minId = 0;
    int maxId = mRenderContainers.count() - 1;

    while(minId <= maxId) {
        const int guess = (minId + maxId)/2;
        const auto& cont = mRenderContainers.at(guess);
        if(cont->inRange(relFrame)) {
            *id = guess;
            return true;
        }
        const int contFrame = cont->getRangeMin();
        if(contFrame > relFrame) {
            if(maxId == guess) {
                *id = minId;
                return mRenderContainers.at(minId)->inRange(relFrame);
            } else {
                maxId = guess;
            }
        } else if(contFrame < relFrame) {
            if(minId == guess) {
                *id = maxId;
                return mRenderContainers.at(maxId)->inRange(relFrame);
            } else {
                minId = guess;
            }
        } else {
            *id = guess;
            return true;
        }
    }
    return false;
}

int HDDCachableCacheHandler::getFirstEmptyFrameAfterFrame(const int &frame) const {
    int currFrame = frame + 1;
    Cont *cont = nullptr;
    while(true) {
        cont = atRelFrame(currFrame);
        if(!cont) return currFrame;
        currFrame = cont->getRangeMax() + 1;
    }
}

int HDDCachableCacheHandler::firstEmptyFrameAtOrAfterFrame(const int &frame) const {
    int currFrame = frame;
    Cont *cont = nullptr;
    while(true) {
        cont = atRelFrame(currFrame);
        if(!cont) return currFrame;
        currFrame = cont->getRangeMax() + 1;
    }
}

void HDDCachableCacheHandler::blockConts(
        const FrameRange &range, const bool &blocked) {
    IdRange idRange = rangeToListIdRange(range);
    for(int i = idRange.fMin; i <= idRange.fMax; i++) {
        mRenderContainers.at(i)->setBlocked(blocked);
    }
}

void HDDCachableCacheHandler::clearCache() {
    mRenderContainers.clear();
}

void HDDCachableCacheHandler::cacheDataBeforeRelFrame(const int &relFrame) {
    const int lastId = idAtOrBeforeRelFrame(relFrame);
    for(int i = 0; i < lastId; i++) {
        mRenderContainers.at(i)->noDataLeft_k();
    }
}

void HDDCachableCacheHandler::cacheDataAfterRelFrame(const int &relFrame) {
    const int firstId = idAtOrAfterRelFrame(relFrame);
    for(int i = firstId; i < mRenderContainers.count(); i++) {
        mRenderContainers.at(i)->noDataLeft_k();
    }
}

void HDDCachableCacheHandler::cacheFirstContainer() {
    if(mRenderContainers.isEmpty()) return;
    mRenderContainers.first()->noDataLeft_k();
}

void HDDCachableCacheHandler::cacheLastContainer() {
    if(mRenderContainers.isEmpty()) return;
    mRenderContainers.last()->noDataLeft_k();
}

int HDDCachableCacheHandler::countAfterRelFrame(const int &relFrame) const {
    int firstId = idAtOrAfterRelFrame(relFrame + 1);
    return mRenderContainers.count() - firstId;
}

int HDDCachableCacheHandler::idAtOrBeforeRelFrame(const int &frame) const {
    int id;
    if(!idAtRelFrame(frame, &id))
        id = insertIdForRelFrame(frame) - 1;
    return id;
}

int HDDCachableCacheHandler::idAtOrAfterRelFrame(const int &frame) const {
    int id;
    if(!idAtRelFrame(frame, &id))
        id = insertIdForRelFrame(frame);
    return id;
}
#include "pointhelpers.h"
void HDDCachableCacheHandler::drawCacheOnTimeline(QPainter * const p,
                                                  const QRectF& drawRect,
                                                  const int &startFrame,
                                                  const int &endFrame,
                                                  const qreal& unit) const {
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
    bool lastStoresInMemory = true;
    const int iMin = qMax(0, insertIdForRelFrame(uStartFrame) - 1);
    for(int i = iMin; i < mRenderContainers.count(); i++) {
        const auto &cont  = mRenderContainers.at(i);
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
        if(storesInMemory != lastStoresInMemory) {
            if(storesInMemory) {
                p->setBrush(QColor(0, 255, 0, 75));
            } else {
                p->setBrush(QColor(0, 0, 255, 75));
            }
            lastStoresInMemory = storesInMemory;
        }

        p->drawRect(xT, qRound(drawRect.top()),
                    widthT, qRound(drawRect.height()));
        lastDrawnFrame = afterMaxFrame;
        lastDrawX = xT + widthT;
    }
    p->restore();
}

void HDDCachableCacheHandler::clearRelRange(const FrameRange &range) {
    int minId = idAtOrAfterRelFrame(range.fMin);
    minId = qMin(minId, mRenderContainers.count() - 1);
    int maxId = idAtOrBeforeRelFrame(range.fMax);
    maxId = qMax(maxId, 0);

    for(int i = maxId; i >= minId; i--) {
        mRenderContainers.removeAt(i);
    }
}

QList<FrameRange> HDDCachableCacheHandler::getMissingRanges(const FrameRange &range) const {
    QList<FrameRange> result;
    int currentFrame = range.fMin;
    while(currentFrame <= range.fMax) {
        auto cont = atOrAfterRelFrame(currentFrame);
        if(!cont) {
            result.append({currentFrame, range.fMax});
            break;
        }
        auto contRange = cont->getRange();
        if(!contRange.inRange(currentFrame)) {
            result.append({currentFrame,
                           qMin(range.fMax, contRange.fMin - 1)});
        }
        currentFrame = contRange.fMax + 1;
    }

    return result;
}

IdRange HDDCachableCacheHandler::rangeToListIdRange(const FrameRange &range) {
    int minId;
    if(!idAtRelFrame(range.fMin, &minId)) {
        minId = insertIdForRelFrame(range.fMin);
    }
    int maxId;
    if(!idAtRelFrame(range.fMax, &maxId)) {
        maxId = insertIdForRelFrame(range.fMax) - 1;
    }
    return {minId, maxId};
}
