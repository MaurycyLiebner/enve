#include "minimalcachehandler.h"
#include "rangecachecontainer.h"

void RangeCacheHandler::removeRenderContainer(
        const stdsptr<RCC> &cont) {
    mRenderContainers.removeOne(cont);
}

int RangeCacheHandler::insertIdForRelFrame(
        const int &relFrame) const {
    int minId = 0;
    int maxId = mRenderContainers.count();

    while(minId < maxId) {
        const int guess = (minId + maxId)/2;
        const auto& cont = mRenderContainers.at(guess);
        const int contFrame = cont->getRangeMin();
        if(contFrame == relFrame)
            RuntimeThrow("Frame already occupied by different container");
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

bool RangeCacheHandler::idAtRelFrame(const int &relFrame, int *id) const {
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

int RangeCacheHandler::getFirstEmptyFrameAfterFrame(const int &frame) const {
    int currFrame = frame + 1;
    RCC *cont = nullptr;
    while(true) {
        cont = atRelFrame(currFrame);
        if(!cont) return currFrame;
        currFrame = cont->getRangeMax() + 1;
    }
}

int RangeCacheHandler::firstEmptyFrameAtOrAfterFrame(const int &frame) const {
    int currFrame = frame;
    RCC *cont = nullptr;
    while(true) {
        cont = atRelFrame(currFrame);
        if(!cont) return currFrame;
        currFrame = cont->getRangeMax() + 1;
    }
}

void RangeCacheHandler::blockConts(
        const FrameRange &range, const bool &blocked) {
    IdRange idRange = rangeToListIdRange(range);
    for(int i = idRange.fMin; i <= idRange.fMax; i++) {
        mRenderContainers.at(i)->setBlocked(blocked);
    }
}

void RangeCacheHandler::clearCache() {
    mRenderContainers.clear();
}

void RangeCacheHandler::cacheDataBeforeRelFrame(const int &relFrame) {
    const int lastId = idAtOrBeforeRelFrame(relFrame);
    for(int i = 0; i < lastId; i++) {
        mRenderContainers.at(i)->noDataLeft_k();
    }
}

void RangeCacheHandler::cacheDataAfterRelFrame(const int &relFrame) {
    const int firstId = idAtOrAfterRelFrame(relFrame);
    for(int i = firstId; i < mRenderContainers.count(); i++) {
        mRenderContainers.at(i)->noDataLeft_k();
    }
}

void RangeCacheHandler::cacheFirstContainer() {
    if(mRenderContainers.isEmpty()) return;
    mRenderContainers.first()->noDataLeft_k();
}

void RangeCacheHandler::cacheLastContainer() {
    if(mRenderContainers.isEmpty()) return;
    mRenderContainers.last()->noDataLeft_k();
}

int RangeCacheHandler::countAfterRelFrame(const int &relFrame) const {
    int firstId = idAtOrAfterRelFrame(relFrame + 1);
    return mRenderContainers.count() - firstId;
}

int RangeCacheHandler::idAtOrBeforeRelFrame(const int &frame) const {
    int id;
    if(!idAtRelFrame(frame, &id)) {
        id = insertIdForRelFrame(frame) - 1;
    }
    return id;
}

int RangeCacheHandler::idAtOrAfterRelFrame(const int &frame) const {
    int id;
    if(!idAtRelFrame(frame, &id)) {
        id = insertIdForRelFrame(frame);
    }
    return id;
}

void RangeCacheHandler::drawCacheOnTimeline(QPainter * const p,
                                            const QRect drawRect,
                                            const int &startFrame,
                                            const int &endFrame) const {
    if(startFrame > endFrame) return;
    p->setBrush(QColor(0, 255, 0, 75));
    p->setPen(Qt::NoPen);
    const qreal pixelsPerFrame = static_cast<qreal>(drawRect.width())/
            (endFrame - startFrame + 1);
    int lastDrawnFrame = startFrame;
    int lastDrawX = 0;
    bool lastStoresInMemory = true;
    for(const auto &cont : mRenderContainers) {
        int afterMaxFrame = cont->getRangeMax() + 1;
        if(afterMaxFrame < startFrame) continue;
        int minFrame = cont->getRangeMin();
        if(minFrame > endFrame + 1) return;

        if(afterMaxFrame > endFrame) afterMaxFrame = endFrame + 1;
        if(minFrame < startFrame) minFrame = startFrame;

        int dFrame = minFrame - startFrame;
        int xT = qRound(dFrame*pixelsPerFrame);

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

        p->drawRect(xT, drawRect.top(), widthT, drawRect.height());
        lastDrawnFrame = afterMaxFrame;
        lastDrawX = xT + widthT;
    }
}

void RangeCacheHandler::clearRelRange(const FrameRange &range) {
    int minId;
    if(!idAtRelFrame(range.fMin, &minId)) {
        minId = insertIdForRelFrame(range.fMin);
    }
    int maxId;
    if(!idAtRelFrame(range.fMax, &maxId)) {
        maxId = insertIdForRelFrame(range.fMax) - 1;
    }
    for(int i = minId; i <= maxId; i++) {
        mRenderContainers.removeAt(minId);
    }
}

QList<FrameRange> RangeCacheHandler::getMissingRanges(const FrameRange &range) const {
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

IdRange RangeCacheHandler::rangeToListIdRange(const FrameRange &range) {
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
