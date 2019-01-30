#include "animationcachehandler.h"

AnimationCacheHandler::AnimationCacheHandler(const QString &filePath) :
    FileCacheHandler(filePath) {}

AnimationCacheHandler::AnimationCacheHandler() :
    FileCacheHandler("") {}
