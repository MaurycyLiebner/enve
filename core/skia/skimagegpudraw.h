#ifndef SKIMAGEGPUDRAW_H
#define SKIMAGEGPUDRAW_H
#include "skiaincludes.h"

extern void drawImageGPU(SkCanvas* const canvas,
                         const sk_sp<SkImage>& image,
                         const SkScalar& x,
                         const SkScalar& y,
                         SkPaint * const paint,
                         GrContext* const context);

#endif // SKIMAGEGPUDRAW_H
