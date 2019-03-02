#ifndef IMAGECACHEHANDLER_H
#define IMAGECACHEHANDLER_H
#include "skia/skiahelpers.h"
#include "filecachehandler.h"

class ImageCacheHandler : public FileCacheHandler {
    friend class StdSelfRef;
public:
    void _processUpdate();
    void afterProcessingFinished();
    void clearCache() {
        mImage.reset();
        FileCacheHandler::clearCache();
    }
    sk_sp<SkImage> getImage() { return mImage; }
    sk_sp<SkImage> getImageCopy() {
        return SkiaHelpers::makeSkImageCopy(mImage);
    }
protected:
    ImageCacheHandler(const QString &filePath,
                      const bool &visibleSeparatly = true);
private:
    sk_sp<SkImage> mUpdateImage;
    sk_sp<SkImage> mImage;
};

#endif // IMAGECACHEHANDLER_H
