#include "animatedsurface.h"

AnimatedSurface::AnimatedSurface(const ushort &widthT,
                                 const ushort &heightT,
                                 const qreal &scale,
                                 const bool &paintOnOtherThread) :
    Surface(widthT, heightT, scale, paintOnOtherThread) {

}

void AnimatedSurface::setCurrentRelFrame(const int &relFrame) {
    mRelFrame = relFrame;
}

Tile*** AnimatedSurface::getSurfaceFrameBeforeRelFrame(
        const int &relFrame) {
    int id;
    if(getSurfaceFrameIdBeforeRelFrame(relFrame, &id)) {
        return mSurfaceFrames.at(id).tiles;
    }
    return NULL;
}

bool AnimatedSurface::getSurfaceFrameIdBeforeRelFrame(const int &relFrame,
                                                      int *id) {
    int minId = 0;
    int maxId = mSurfaceFrames.count() - 1;

    while(minId <= maxId) {
        int guess = (minId + maxId)/2;
        const SurfaceFrame &frameT = mSurfaceFrames.at(guess);
        if(frameT.relFrame >= relFrame) {
            maxId = guess - 1;
            continue;
        }
        if(guess == mSurfaceFrames.count() - 1) {
            *id = guess;
            return true;
        }
        const SurfaceFrame &nextFrameT = mSurfaceFrames.at(guess + 1);
        if(nextFrameT.relFrame >= relFrame) {
            *id = guess;
            return true;
        }
        minId = guess + 1;
    }
    return false;
}

Tile*** AnimatedSurface::getTilesAfterRelFrame(
        const int &relFrame) {
    int id;
    if(getSurfaceFrameIdBeforeRelFrame(relFrame, &id)) {
        return mSurfaceFrames.at(id).tiles;
    }
    return NULL;
}

bool AnimatedSurface::getSurfaceFrameIdAfterRelFrame(const int &relFrame,
                                                     int *id) {
    int minId = 0;
    int maxId = mSurfaceFrames.count() - 1;

    while(minId <= maxId) {
        int guess = (minId + maxId)/2;
        const SurfaceFrame &frameT = mSurfaceFrames.at(guess);
        if(frameT.relFrame <= relFrame) {
            minId = guess + 1;
            continue;
        }
        if(guess == 0) {
            *id = guess;
            return true;
        }
        const SurfaceFrame &prevFrameT = mSurfaceFrames.at(guess - 1);
        if(prevFrameT.relFrame <= relFrame) {
            *id = guess;
            return true;
        }
        maxId = guess - 1;
    }
    return false;
}

void AnimatedSurface::setCurrentSurfaceFrame(
        const SurfaceFrame &surfaceFrame) {
    mCurrentSurfaceFrame = surfaceFrame;
    mCurrentTiles = surfaceFrame.tiles;
}
