// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef IMAGECACHECONTAINER_H
#define IMAGECACHECONTAINER_H
#include "skia/skiaincludes.h"
#include "hddcachablerangecont.h"
class Canvas;

class ImageCacheContainer : public HddCachableRangeCont {
    e_OBJECT
protected:
    ImageCacheContainer(const FrameRange& range,
                        HddCachableCacheHandler * const parent);
    ImageCacheContainer(const sk_sp<SkImage>& img,
                        const FrameRange &range,
                        HddCachableCacheHandler * const parent);
    stdsptr<eHddTask> createTmpFileDataSaver();
    stdsptr<eHddTask> createTmpFileDataLoader();
    int clearMemory();
public:
    int getByteCount();

    void drawSk(SkCanvas * const canvas, const SkFilterQuality filter);

    sk_sp<SkImage> getImageSk();

    void setDataLoadedFromTmpFile(const sk_sp<SkImage> &img);
    void replaceImageSk(const sk_sp<SkImage> &img);
protected:
    sk_sp<SkImage> mImageSk;
};


#endif // IMAGECACHECONTAINER_H
