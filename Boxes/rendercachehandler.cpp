#include "rendercachehandler.h"
#include "boundingboxrendercontainer.h"
#include "key.h"
#include "Animators/complexanimator.h"
#include "durationrectangle.h"
#include "global.h"

void CacheHandler::removeRenderContainer(const CacheContainerSPtr& cont) {
    for(int i = 0; i < mRenderContainers.count(); i++) {
        if(mRenderContainers.at(i) == cont) {
            mRenderContainers.removeAt(i);
        }
    }
}

bool CacheHandler::getRenderContainterIdAtRelFrame(const int &relFrame,
                                                   int *id) {
    int minId = 0;
    int maxId = mRenderContainers.count() - 1;

    while(minId <= maxId) {
        int guess = (minId + maxId)/2;
        CacheContainer *cont = mRenderContainers.at(guess).get();
        if(cont->relFrameInRange(relFrame)) {
            *id = guess;
            return true;
        }
        int contFrame = cont->getMinRelFrame();
        if(contFrame > relFrame) {
            if(maxId == guess) {
                *id = minId;
                return mRenderContainers.at(minId)->relFrameInRange(relFrame);
            } else {
                maxId = guess;
            }
        } else if(contFrame < relFrame) {
            if(minId == guess) {
                *id = maxId;
                return mRenderContainers.at(maxId)->relFrameInRange(relFrame);
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

int CacheHandler::getRenderContainterInsertIdAtRelFrame(
                    const int &relFrame) {
    int minId = 0;
    int maxId = mRenderContainers.count();

    while(minId < maxId) {
        int guess = (minId + maxId)/2;
        CacheContainer *cont = mRenderContainers.at(guess).get();
        int contFrame = cont->getMinRelFrame();
        Q_ASSERT(contFrame != relFrame);
        if(contFrame > relFrame) {
            if(guess == maxId) {
                return minId;
            }
            maxId = guess;
        } else if(contFrame < relFrame) {
            if(guess == minId) {
                return maxId;
            }
            minId = guess;
        }
    }
    return 0;
}

CacheContainer *CacheHandler::createNewRenderContainerAtRelFrame(const int &frame) {
    CacheContainerSPtr cont = SPtrCreate(CacheContainer)(this);
    cont->setRelFrame(frame);
    int contId = getRenderContainterInsertIdAtRelFrame(frame);
    mRenderContainers.insert(contId, getAsSPtr(cont, CacheContainer));
    return cont.get();
}

int CacheHandler::getFirstEmptyOrCachedFrameAfterFrame(const int &frame,
                                                       CacheContainer **contP) {
    int currFrame = frame + 1;
    CacheContainer *cont = nullptr;
    while(true) {
        cont = getRenderContainerAtRelFrame(currFrame);
        if(cont == nullptr) {
            break;
        } else if(!cont->storesDataInMemory()) {
            break;
        }
        currFrame = cont->getMaxRelFrame() + 1;
    }
    if(contP != nullptr) {
        *contP = cont;
    }
    return currFrame;
}

int CacheHandler::getFirstEmptyFrameAfterFrame(const int &frame) {
    int currFrame = frame + 1;
    CacheContainer *cont = nullptr;
    while(true) {
        cont = getRenderContainerAtRelFrame(currFrame);
        if(cont == nullptr) {
            return currFrame;
        }
        currFrame = cont->getMaxRelFrame() + 1;
    }
}

int CacheHandler::getFirstEmptyFrameAtOrAfterFrame(const int &frame) {
    int currFrame = frame;
    CacheContainer *cont = nullptr;
    while(true) {
        cont = getRenderContainerAtRelFrame(currFrame);
        if(cont == nullptr) {
            return currFrame;
        }
        currFrame = cont->getMaxRelFrame() + 1;
    }
}

void CacheHandler::setContainersInFrameRangeBlocked(const int &minFrame,
                                                    const int &maxFrame,
                                                    const bool &blocked) {
    int minId;
    if(!getRenderContainterIdAtRelFrame(minFrame, &minId)) {
        minId = getRenderContainterInsertIdAtRelFrame(minFrame);
    }
    int maxId;
    if(!getRenderContainterIdAtRelFrame(maxFrame, &maxId)) {
        maxId = getRenderContainterInsertIdAtRelFrame(maxFrame) - 1;
    }
    for(int i = minId; i <= maxId; i++) {
        mRenderContainers.at(i)->setBlocked(blocked);
    }
}

void CacheHandler::clearCache() {
    mRenderContainers.clear();
}

void CacheHandler::cacheDataBeforeRelFrame(const int &relFrame) {
    int lastId = getRenderContainerIdAtOrBeforeRelFrame(relFrame);
    for(int i = 0; i < lastId; i++) {
        mRenderContainers.at(i)->cacheFreeAndRemoveFromMemoryHandler();
    }
}

void CacheHandler::cacheDataAfterRelFrame(const int &relFrame) {
    int firstId = getRenderContainerIdAtOrAfterRelFrame(relFrame);
    for(int i = firstId; i < mRenderContainers.count(); i++) {
        mRenderContainers.at(i)->cacheFreeAndRemoveFromMemoryHandler();
    }
}

void CacheHandler::cacheFirstContainer() {
    if(mRenderContainers.isEmpty()) return;
    mRenderContainers.first()->cacheFreeAndRemoveFromMemoryHandler();
}

void CacheHandler::cacheLastContainer() {
    if(mRenderContainers.isEmpty()) return;
    mRenderContainers.last()->cacheFreeAndRemoveFromMemoryHandler();
}

int CacheHandler::getContainerCountAfterRelFrame(const int &relFrame) {
    int firstId = getRenderContainerIdAtOrAfterRelFrame(relFrame + 1);
    return mRenderContainers.count() - firstId;
}

int CacheHandler::getNumberNotCachedBeforeRelFrame(const int &relFrame) {
    int nNotCached = 0;
    int firstId = getRenderContainerIdAtOrBeforeRelFrame(relFrame - 1);
    for(int i = 0; i < firstId; i++) {
        if(mRenderContainers.at(i)->storesDataInMemory()) {
            nNotCached++;
        }
    }
    return nNotCached;
}
#include "memoryhandler.h"
void CacheHandler::updateAllAfterFrameInMemoryHandler(const int &relFrame) {
    int firstId = getRenderContainerIdAtOrAfterRelFrame(relFrame + 1);
    for(int i = mRenderContainers.count() - 1; i >= firstId; i--) {
        CacheContainer *cont = mRenderContainers.at(i).get();
        if(cont->handledByMemoryHandler()) {
            MemoryHandler::getInstance()->containerUpdated(cont);
        }
    }
}

CacheContainer *CacheHandler::getRenderContainerAtRelFrame(const int &frame) {
    int id;
    if(getRenderContainterIdAtRelFrame(frame, &id)) {
        return mRenderContainers.at(id).get();
    }
    return nullptr;
}

int CacheHandler::getRenderContainerIdAtOrBeforeRelFrame(const int &frame) {
    int id;
    if(!getRenderContainterIdAtRelFrame(frame, &id)) {
        id = getRenderContainterInsertIdAtRelFrame(frame) - 1;
    }
    return id;
}

CacheContainer *CacheHandler::getRenderContainerAtOrBeforeRelFrame(
                                                const int &frame) {
    CacheContainer *cont = getRenderContainerAtRelFrame(frame);
    if(cont == nullptr) {
        int id = getRenderContainterInsertIdAtRelFrame(frame) - 1;
        if(id >= 0 && id < mRenderContainers.length()) {
            cont = mRenderContainers.at(id).get();
        }
    }
    return cont;
}


int CacheHandler::getRenderContainerIdAtOrAfterRelFrame(const int &frame) {
    int id;
    if(!getRenderContainterIdAtRelFrame(frame, &id)) {
        id = getRenderContainterInsertIdAtRelFrame(frame);
    }
    return id;
}

CacheContainer *CacheHandler::getRenderContainerAtOrAfterRelFrame(
                                                const int &frame) {
    CacheContainer *cont = getRenderContainerAtRelFrame(frame);
    if(cont == nullptr) {
        int id = getRenderContainterInsertIdAtRelFrame(frame);
        if(id >= 0 && id < mRenderContainers.length()) {
            cont = mRenderContainers.at(id).get();
        }
    }
    return cont;
}

void CacheHandler::drawCacheOnTimeline(QPainter *p,
                           const qreal &pixelsPerFrame,
                           const qreal &drawY,
                           const int &startFrame,
                           const int &endFrame) {
    p->setBrush(QColor(0, 255, 0, 75));
    p->setPen(Qt::NoPen);
    int lastDrawnFrame = startFrame;
    int lastDrawX = 0;
    bool lastStoresInMemory = true;
    Q_FOREACH(const CacheContainerSPtr &cont, mRenderContainers) {
        int afterMaxFrame = cont->getMaxRelFrame() + 1;
        if(afterMaxFrame < startFrame) continue;
        int minFrame = cont->getMinRelFrame();
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
        bool storesInMemory = cont->storesDataInMemory();
        if(storesInMemory != lastStoresInMemory) {
            if(storesInMemory) {
                p->setBrush(QColor(0, 255, 0, 75));
            } else {
                p->setBrush(QColor(0, 0, 255, 75));
            }
            lastStoresInMemory = storesInMemory;
        }

        p->drawRect(xT, qRound(drawY), widthT, MIN_WIDGET_HEIGHT);
        lastDrawnFrame = afterMaxFrame;
        lastDrawX = xT + widthT;
    }
}

void CacheHandler::clearCacheForRelFrameRange(const int &minFrame,
                                              const int &maxFrame) {
    int minId;
    if(!getRenderContainterIdAtRelFrame(minFrame, &minId)) {
        minId = getRenderContainterInsertIdAtRelFrame(minFrame);
    }
    int maxId;
    if(!getRenderContainterIdAtRelFrame(maxFrame, &maxId)) {
        maxId = getRenderContainterInsertIdAtRelFrame(maxFrame) - 1;
    }
    for(int i = minId; i <= maxId; i++) {
        mRenderContainers.removeAt(minId);
    }
}
