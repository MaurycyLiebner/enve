#include "skimagegpudraw.h"
#include <QDebug>

void drawImageGPU(SkCanvas* const canvas,
                  const sk_sp<SkImage>& image,
                  const SkScalar& x,
                  const SkScalar& y,
                  SkPaint * const paint,
                  GrContext* const context) {
    if(nullptr == image || nullptr == context) {
        return;
    }
    sk_sp<SkImage> texture(image->makeTextureImage(context, nullptr));
    GrGLTextureInfo glTexInfo;
    texture->getBackendTexture(false).getGLTextureInfo(&glTexInfo);
    canvas->drawImage(texture, x, y, paint);
};
