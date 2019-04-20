#ifndef RENDERCACHEHANDLER_H
#define RENDERCACHEHANDLER_H
#include <QtCore>
#include <memory>
#include <QPainter>
#include "memoryhandler.h"
#include "smartPointers/sharedpointerdefs.h"
#include "framerange.h"
#include "boundingboxrendercontainer.h"
#include "global.h"
#include "CacheHandlers/hddcachablecachehandler.h"

class QPainter;

class RenderCacheHandler : public HDDCachableCacheHandler {
public:
};

#endif // RENDERCACHEHANDLER_H
