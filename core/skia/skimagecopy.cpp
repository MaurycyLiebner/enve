#include "skimagecopy.h"

sk_sp<SkImage> makeSkImageCopy(const sk_sp<SkImage>& img) {
    if(img.get() == nullptr) return sk_sp<SkImage>();
    SkPixmap pix;
    if(!img->peekPixels(&pix)) {
        return img->makeRasterImage();
    }
    return SkImage::MakeRasterCopy(pix);
}
