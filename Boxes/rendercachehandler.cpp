#include "rendercachehandler.h"
#include "boundingboxrendercontainer.h"
#include "key.h"
#include "Animators/complexanimator.h"
#include "durationrectangle.h"
#include "global.h"

void CacheHandler::removeRenderContainer(CacheContainer *cont) {
    mRenderContainers.removeOne(cont);
    cont->setParentCacheHandler(NULL);
    cont->decNumberPointers();
}

bool CacheHandler::getRenderContainterIdAtRelFrame(const int &relFrame,
                                                   int *id) {
    int minId = 0;
    int maxId = mRenderContainers.count() - 1;

    while(minId <= maxId) {
        int guess = (minId + maxId)/2;
        CacheContainer *cont = mRenderContainers.at(guess);
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

int CacheHandler::getRenderContainterInsertIdAtRelFrame(const int &relFrame) {
    int minId = 0;
    int maxId = mRenderContainers.count();

    while(minId < maxId) {
        int guess = (minId + maxId)/2;
        CacheContainer *cont = mRenderContainers.at(guess);
        int contFrame = cont->getMinRelFrame();
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
    CacheContainer *cont = new CacheContainer();
    cont->setParentCacheHandler(this);
    cont->setRelFrame(frame);
    cont->incNumberPointers();
    int contId = getRenderContainterInsertIdAtRelFrame(frame);
    mRenderContainers.insert(contId, cont);
    return cont;
}

int CacheHandler::getFirstEmptyFrameAfterFrame(const int &frame) {
    int currFrame = frame + 1;
    CacheContainer *cont = NULL;
    while(true) {
        cont = getRenderContainerAtRelFrame(currFrame);
        if(cont == NULL) {
            return currFrame;
        }
        currFrame = cont->getMaxRelFrame();
    }
    return currFrame;
}

int CacheHandler::getFirstEmptyFrameAtOrAfterFrame(const int &frame) {
    int currFrame = frame;
    CacheContainer *cont = NULL;
    while(true) {
        cont = getRenderContainerAtRelFrame(currFrame);
        if(cont == NULL) {
            return currFrame;
        }
        currFrame = cont->getMaxRelFrame();
    }
    return currFrame;
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
    foreach(CacheContainer *cont, mRenderContainers) {
        cont->setParentCacheHandler(NULL);
        cont->decNumberPointers();
    }

    mRenderContainers.clear();
}

CacheContainer *CacheHandler::getRenderContainerAtRelFrame(const int &frame) {
    int id;
    if(getRenderContainterIdAtRelFrame(frame, &id)) {
        return mRenderContainers.at(id);
    }
    return NULL;
}

CacheContainer *CacheHandler::getRenderContainerAtOrBeforeRelFrame(
                                                const int &frame) {
    CacheContainer *cont = getRenderContainerAtRelFrame(frame);
    if(cont == NULL) {
        int id = getRenderContainterInsertIdAtRelFrame(frame) - 1;
        if(id >= 0 && id < mRenderContainers.length()) {
            cont = mRenderContainers.at(id);
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
    foreach(CacheContainer *cont, mRenderContainers) {
        int maxFrame = cont->getMaxRelFrame();
        int minFrame = cont->getMinRelFrame();
        if(maxFrame < startFrame) continue;
        if(minFrame > endFrame + 1) return;

        if(maxFrame > endFrame) maxFrame = endFrame + 1;
        if(minFrame < startFrame) minFrame = startFrame;

        int dFrame = minFrame - startFrame;
        int xT = dFrame*pixelsPerFrame;

        int widthT = pixelsPerFrame*(maxFrame - minFrame);
        if(lastDrawnFrame == minFrame) {
            widthT += xT - lastDrawX;
            xT = lastDrawX;
        }
        p->drawRect(xT, drawY, widthT, MIN_WIDGET_HEIGHT);
        lastDrawnFrame = maxFrame;
        lastDrawX = xT + widthT;
    }
}

RenderCacheHandler::RenderCacheHandler() {

}

void RenderCacheHandler::clearCacheForRelFrameRange(const int &minFrame,
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
        CacheContainer *cont = mRenderContainers.takeAt(minId);
        cont->setParentCacheHandler(NULL);
        cont->decNumberPointers();
    }
    int minFrameT = minFrame;
    int maxFrameT = maxFrame;
    relRangeToAbsRange(&minFrameT, &maxFrameT);
    emit clearedCacheForAbsFrameRange(minFrameT, maxFrameT);
}

#include <QDebug>
CacheContainer *RenderCacheHandler::createNewRenderContainerAtRelFrame(const int &frame) {
    RenderContainer *cont = new RenderContainer();
    cont->setParentCacheHandler(this);
    cont->setRelFrame(frame);
    cont->incNumberPointers();
    int contId = getRenderContainterInsertIdAtRelFrame(frame);
    mRenderContainers.insert(contId, cont);
    return cont;
}

#include "boundingbox.h"
int RenderCacheHandler::relFrameToAbsFrame(const int &relFrame) {
    return mParentBox->prp_relFrameToAbsFrame(relFrame);
}

int RenderCacheHandler::absFrameToRelFrame(const int &absFrame) {
    return mParentBox->prp_absFrameToRelFrame(absFrame);
}

void RenderCacheHandler::clearCacheForAbsFrameRange(const int &minFrame,
                                                    const int &maxFrame) {
    int minFrameT = minFrame;
    int maxFrameT = maxFrame;
    absRangeToRelRange(&minFrameT, &maxFrameT);
    clearCacheForRelFrameRange(minFrameT, maxFrameT);
}
